#include "TsneAnalysis.h"

#include "hdi/utils/glad/glad.h"
#include "OffscreenBuffer.h"

#include <vector>
#include <assert.h>

#include <QDebug>
#include <QCoreApplication>

#include "hdi/dimensionality_reduction/tsne_parameters.h"
#include "hdi/utils/scoped_timers.h"

using namespace mv;

TsneWorker::TsneWorker(TsneParameters parameters, const std::vector<hdi::data::MapMemEff<uint32_t, float>>& probDist, int numPoints, int numDimensions) :
    _currentIteration(0),
    _parameters(parameters),
    _probabilityDistribution(probDist),
    _numPoints(numPoints),
    _numDimensions(numDimensions),
    _hasProbabilityDistribution(true),
    _offscreenBuffer(nullptr),
    _shouldStop(false),
    _parentTask(nullptr),
    _tasks(nullptr)
    
{
    // Offscreen buffer must be created in the UI thread because it is a QWindow, afterwards we move it
    _offscreenBuffer = new OffscreenBuffer();
}

TsneWorker::TsneWorker(TsneParameters parameters, /*const*/ std::vector<float>& data, int numDimensions) :
    _currentIteration(0),
    _parameters(parameters),
    _data(data),
    _numPoints(data.size() / numDimensions),
    _numDimensions(numDimensions),
    _hasProbabilityDistribution(false),
    _offscreenBuffer(nullptr),
    _shouldStop(false),
    _parentTask(nullptr),
    _tasks(nullptr)
{
    // Offscreen buffer must be created in the UI thread because it is a QWindow, afterwards we move it
    _offscreenBuffer = new OffscreenBuffer();
}

TsneWorker::~TsneWorker()
{
    delete _offscreenBuffer;
}

void TsneWorker::changeThread(QThread* targetThread)
{
    this->moveToThread(targetThread);
    
    //_task->moveToThread(targetThread);

    

    // Move the Offscreen buffer to the processing thread after creating it in the UI Thread
    _offscreenBuffer->moveToThread(targetThread);
}

int TsneWorker::getNumIterations() const
{
    return _currentIteration + 1;
}

void TsneWorker::setParentTask(mv::Task* parentTask)
{
    _parentTask = parentTask;

    
    //_tasks->getComputeGradientDescentTask().setGuiScopes({ Task::GuiScope::Foreground });
}

void TsneWorker::createTasks()
{
    _tasks = new TsneWorkerTasks(this, _parentTask);
}

void TsneWorker::computeSimilarities()
{
    _tasks->getComputingSimilaritiesTask().setRunning();

    hdi::dr::HDJointProbabilityGenerator<float>::Parameters probGenParams;

    probGenParams._perplexity               = _parameters.getPerplexity();
    probGenParams._perplexity_multiplier    = 3;
    probGenParams._num_trees                = _parameters.getNumTrees();
    probGenParams._num_checks               = _parameters.getNumChecks();
    probGenParams._aknn_algorithm           = _parameters.getKnnAlgorithm();
    probGenParams._aknn_metric              = _parameters.getKnnDistanceMetric();

    qDebug() << "tSNE initialized.";

    _probabilityDistribution.clear();
    _probabilityDistribution.resize(_numPoints);

    qDebug() << "Sparse matrix allocated.";
    qDebug() << "Computing high dimensional probability distributions.. Num dims: " << _numDimensions << " Num data points: " << _numPoints;

    hdi::dr::HDJointProbabilityGenerator<float> probabilityGenerator;
    double t = 0.0;
    {
        hdi::utils::ScopedTimer<double> timer(t);
        // The _probabilityDistribution is symmetrized here.
        probabilityGenerator.computeJointProbabilityDistribution(_data.data(), _numDimensions, _numPoints, _probabilityDistribution, probGenParams);
    }
        
    qDebug() << "Probability distributions calculated.";
    qDebug() << "================================================================================";
    qDebug() << "A-tSNE: Compute probability distribution: " << t / 1000 << " seconds";
    qDebug() << "--------------------------------------------------------------------------------";

    _tasks->getComputingSimilaritiesTask().setFinished();
}

void TsneWorker::computeGradientDescent(int iterations)
{
    if (_shouldStop)
        return;

    _tasks->getInitializeTsneTask().setRunning();

    //_currentIteration = 0;

    hdi::dr::TsneParameters tsneParameters;

    tsneParameters._embedding_dimensionality    = _parameters.getNumDimensionsOutput();
    tsneParameters._mom_switching_iter          = _parameters.getExaggerationIter();
    tsneParameters._remove_exaggeration_iter    = _parameters.getExaggerationIter();
    tsneParameters._exponential_decay_iter      = _parameters.getExponentialDecayIter();
    tsneParameters._exaggeration_factor         = 4 + _numPoints / 60000.0;

    // Initialize offscreen buffer
    double t_buffer = 0.0;
    {
        hdi::utils::ScopedTimer<double> timer(t_buffer);
        _offscreenBuffer->bindContext();
    }
    qDebug() << "A-tSNE: Set up offscreen buffer in " << t_buffer / 1000 << " seconds.";

    // Initialize GPGPU-SNE
    double t_init = 0.0;
    {
        hdi::utils::ScopedTimer<double> timer(t_init);

        if (_currentIteration == 0)
        {
            // In case of HSNE, the _probabilityDistribution is a non-summetric transition matrix and initialize() symmetrizes it here
            if (_hasProbabilityDistribution)
                _GPGPU_tSNE.initialize(_probabilityDistribution, &_embedding, tsneParameters);
            else
                _GPGPU_tSNE.initializeWithJointProbabilityDistribution(_probabilityDistribution, &_embedding, tsneParameters);
        }
    }

    qDebug() << "A-tSNE: Init t-SNE " << t_init / 1000 << " seconds.";

    const auto updateEmbedding = [this](const TsneData& tsneData) -> void {
        copyEmbeddingOutput();
        emit embeddingUpdate(tsneData);
    };

    updateEmbedding(_outEmbedding);

    _tasks->getInitializeTsneTask().setFinished();

    double elapsed = 0;
    double t_grad = 0;
    {
        qDebug() << "A-tSNE: Computing gradient descent..";

        _tasks->getComputeGradientDescentTask().setRunning();
        _tasks->getComputeGradientDescentTask().setSubtasks(iterations);

        const auto beginIteration   = _currentIteration;
        const auto endIteration     = beginIteration + iterations;

        int currentStepIndex = 0;

        // Performs gradient descent for every iteration
        for (_currentIteration = beginIteration; _currentIteration < endIteration; ++_currentIteration) {

            _tasks->getComputeGradientDescentTask().setSubtaskStarted(currentStepIndex);// , "sdsad");// QString("Step %1").arg(QString::number(_currentIteration)));

            hdi::utils::ScopedTimer<double> timer(t_grad);

            // Perform a GPGPU-SNE iteration
            _GPGPU_tSNE.doAnIteration();

            if (_currentIteration > 0 && _parameters.getUpdateCore() > 0 && _currentIteration % _parameters.getUpdateCore() == 0)
                updateEmbedding(_outEmbedding);

            if (t_grad > 1000)
                qDebug() << "Time: " << t_grad;

            elapsed += t_grad;

            // React to requests to stop
            if (_shouldStop)
                break;
            
            _tasks->getComputeGradientDescentTask().setSubtaskFinished(currentStepIndex);

            currentStepIndex++;

            QCoreApplication::processEvents();
        }

        _offscreenBuffer->releaseContext();

        updateEmbedding(_outEmbedding);

        _tasks->getComputeGradientDescentTask().setFinished();
    }

    qDebug() << "--------------------------------------------------------------------------------";
    qDebug() << "A-tSNE: Finished embedding of " << "tSNE Analysis" << " in: " << elapsed / 1000 << " seconds ";
    qDebug() << "================================================================================";

    emit finished();
}

void TsneWorker::copyEmbeddingOutput()
{
    _outEmbedding.assign(_numPoints, _parameters.getNumDimensionsOutput(), _embedding.getContainer());
}

void TsneWorker::compute()
{
    createTasks();

    connect(_parentTask, &Task::requestAbort, this, [this]() -> void { _shouldStop = true; }, Qt::DirectConnection);

    _shouldStop = false;

    double t = 0.0;
    {
        hdi::utils::ScopedTimer<double> timer(t);

        _tasks->getInitializeOffScreenBufferTask().setRunning();

        // Create a context local to this thread that shares with the global share context
        _offscreenBuffer->initialize();
        
        _tasks->getInitializeOffScreenBufferTask().setFinished();

        if (!_hasProbabilityDistribution)
            computeSimilarities();

        computeGradientDescent(_parameters.getNumIterations());
    }
    qDebug() << "t-SNE total compute time: " << t / 1000 << " seconds.";

    if (_shouldStop)
        _tasks->getComputeGradientDescentTask().setAborted();
    else
        _tasks->getComputeGradientDescentTask().setFinished();

    _parentTask->setFinished();
}

void TsneWorker::continueComputation(int iterations)
{
    _tasks->getInitializeOffScreenBufferTask().setEnabled(false);
    _tasks->getComputingSimilaritiesTask().setEnabled(false);
    _tasks->getInitializeTsneTask().setEnabled(false);
    
    connect(_parentTask, &Task::requestAbort, this, [this]() -> void { _shouldStop = true; }, Qt::DirectConnection);

    _shouldStop = false;

    double t = 0.0;
    {
        hdi::utils::ScopedTimer<double> timer(t);
        computeGradientDescent(iterations);
    }

    _parentTask->setFinished();
}

void TsneWorker::stop()
{
    _shouldStop = true;
}

TsneAnalysis::TsneAnalysis() :
    _tsneWorker(nullptr),
    _task(nullptr)
{
    qRegisterMetaType<TsneData>();
}

TsneAnalysis::~TsneAnalysis()
{
}

void TsneAnalysis::startComputation(TsneParameters parameters, const std::vector<hdi::data::MapMemEff<uint32_t, float>>& probDist, int numPoints, int numDimensions)
{
    if (_tsneWorker)
    {
        _tsneWorker->changeThread(QThread::currentThread());
        delete _tsneWorker;
    }

    _tsneWorker = new TsneWorker(parameters, probDist, numPoints, numDimensions);

    _tsneWorker->setParentTask(_task);

    startComputation(_tsneWorker);
}

void TsneAnalysis::startComputation(TsneParameters parameters, /*const*/ std::vector<float>& data, int numDimensions)
{
    if (_tsneWorker)
    {
        _tsneWorker->changeThread(QThread::currentThread());
        delete _tsneWorker;
    }

    _tsneWorker = new TsneWorker(parameters, data, numDimensions);
    
    _tsneWorker->setParentTask(_task);

    startComputation(_tsneWorker);
}

void TsneAnalysis::continueComputation(int iterations)
{
    emit continueWorker(iterations);
}

void TsneAnalysis::stopComputation()
{
    emit stopWorker();

    /*
    _workerThread.exit();

    // Wait until the thread has terminated (max. 3 seconds)
    if (!_workerThread.wait(3000))
    {
        qDebug() << "tSNE computation thread did not close in time, terminating...";
        _workerThread.terminate();
        _workerThread.wait();
    }
    qDebug() << "tSNE computation stopped.";
    */

    emit aborted();
}

bool TsneAnalysis::canContinue() const
{
    if (_tsneWorker == nullptr)
        return false;
    
    return _tsneWorker->getNumIterations() >= 1;
}

int TsneAnalysis::getNumIterations() const
{
    return _tsneWorker->getNumIterations();
}

void TsneAnalysis::setTask(mv::Task* task)
{
    _task = task;
}

void TsneAnalysis::startComputation(TsneWorker* tsneWorker)
{
    tsneWorker->changeThread(&_workerThread);

    // To-Worker signals
    connect(this, &TsneAnalysis::startWorker, tsneWorker, &TsneWorker::compute);
    connect(this, &TsneAnalysis::continueWorker, tsneWorker, &TsneWorker::continueComputation);
    connect(this, &TsneAnalysis::stopWorker, tsneWorker, &TsneWorker::stop, Qt::DirectConnection);

    // From-Worker signals
    connect(tsneWorker, &TsneWorker::embeddingUpdate, this, &TsneAnalysis::embeddingUpdate);
    connect(tsneWorker, &TsneWorker::finished, this, &TsneAnalysis::finished);

    _workerThread.start();

    emit startWorker();
}

TsneWorkerTasks::TsneWorkerTasks(QObject* parent, mv::Task* parentTask) :
    QObject(parent),
    _initializeOffScreenBufferTask(this, "Initializing off-screen GP-GPU buffer", Task::GuiScopes{ Task::GuiScope::DataHierarchy, Task::GuiScope::Foreground }, Task::Status::Idle),
    _computingSimilaritiesTask(this, "Computing similarities", Task::GuiScopes{ Task::GuiScope::DataHierarchy, Task::GuiScope::Foreground }, Task::Status::Idle),
    _initializeTsneTask(this, "Initialize TSNE", Task::GuiScopes{ Task::GuiScope::DataHierarchy, Task::GuiScope::Foreground }, Task::Status::Idle),
    _computeGradientDescentTask(this, "Computing gradient descent", Task::GuiScopes{ Task::GuiScope::DataHierarchy, Task::GuiScope::Foreground }, Task::Status::Idle)
{
    _initializeOffScreenBufferTask.setParentTask(parentTask);
    _computingSimilaritiesTask.setParentTask(parentTask);
    _initializeTsneTask.setParentTask(parentTask);
    _computeGradientDescentTask.setParentTask(parentTask);

    _computeGradientDescentTask.setSubtaskNamePrefix("Compute gradient descent step");

    /*
    _initializeOffScreenBufferTask.moveToThread(targetThread);
    _computingSimilaritiesTask.moveToThread(targetThread);
    _initializeTsneTask.moveToThread(targetThread);
    _computeGradientDescentTask.moveToThread(targetThread);
    */
}
