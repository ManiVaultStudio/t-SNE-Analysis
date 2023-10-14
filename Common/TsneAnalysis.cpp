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
    _foregroundTask(nullptr)
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
    _foregroundTask(nullptr)
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
    // Move the Offscreen buffer to the processing thread after creating it in the UI Thread
    _offscreenBuffer->moveToThread(targetThread);
}

int TsneWorker::getNumIterations() const
{
    return _currentIteration + 1;
}

void TsneWorker::setForegroundTask(ForegroundTask* foregroundTask)
{
    _foregroundTask = foregroundTask;
}

void TsneWorker::computeSimilarities()
{
    _foregroundTask->setProgressDescription("Initializing tSNE");

    hdi::dr::HDJointProbabilityGenerator<float>::Parameters probGenParams;
    probGenParams._perplexity = _parameters.getPerplexity();
    probGenParams._perplexity_multiplier = 3;
    probGenParams._num_trees = _parameters.getNumTrees();
    probGenParams._num_checks = _parameters.getNumChecks();
    probGenParams._aknn_algorithm = _parameters.getKnnAlgorithm();
    probGenParams._aknn_metric = _parameters.getKnnDistanceMetric();

    qDebug() << "tSNE initialized.";

    _foregroundTask->setProgressDescription("tSNE initialized");
    _foregroundTask->setProgressDescription("Calculate probability distributions");

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

    _foregroundTask->setProgressDescription("Probability distributions calculated");
        
    qDebug() << "Probability distributions calculated.";
    qDebug() << "================================================================================";
    qDebug() << "A-tSNE: Compute probability distribution: " << t / 1000 << " seconds";
    qDebug() << "--------------------------------------------------------------------------------";
}

void TsneWorker::computeGradientDescent(int iterations)
{
    if (_shouldStop)
        return;

    _foregroundTask->setProgressDescription("Initializing gradient descent");
    
    QCoreApplication::processEvents();

    //_currentIteration = 0;

    hdi::dr::TsneParameters tsneParameters;

    tsneParameters._embedding_dimensionality = _parameters.getNumDimensionsOutput();
    tsneParameters._mom_switching_iter = _parameters.getExaggerationIter();
    tsneParameters._remove_exaggeration_iter = _parameters.getExaggerationIter();
    tsneParameters._exponential_decay_iter = _parameters.getExponentialDecayIter();
    tsneParameters._exaggeration_factor = 4 + _numPoints / 60000.0;

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

    _foregroundTask->setProgressDescription(QString("Initialization took %1 seconds").arg(QString::number(t_init / 1000, 'f', 1)));

    const auto updateEmbedding = [this](const TsneData& tsneData) -> void {
        copyEmbeddingOutput();
        emit embeddingUpdate(tsneData);
    };

    updateEmbedding(_outEmbedding);

    double elapsed = 0;
    double t_grad = 0;
    {
        qDebug() << "A-tSNE: Computing gradient descent..";

        _foregroundTask->setProgressDescription("Computing gradient descent");

        const auto beginIteration   = _currentIteration;
        const auto endIteration     = beginIteration + iterations;

        int currentStepIndex = 0;

        // Performs gradient descent for every iteration
        for (_currentIteration = beginIteration; _currentIteration < endIteration; ++_currentIteration)
        {
            const auto subtask = QString("Step %1").arg(QString::number(currentStepIndex));

            _foregroundTask->setSubtaskStarted(subtask);
            {
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
            }
            _foregroundTask->setSubtaskFinished(subtask);

            currentStepIndex++;

            QCoreApplication::processEvents();
        }

        _offscreenBuffer->releaseContext();

        updateEmbedding(_outEmbedding);
    }

    _foregroundTask->setProgressDescription("Finished embedding");

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
    QStringList subtasks{ "Compute similarities" };

    for (int stepIndex = 0; stepIndex < _parameters.getNumIterations(); stepIndex++)
        subtasks << QString("Step %1").arg(QString::number(stepIndex));
    
    _foregroundTask->setSubtasks(subtasks);

    connect(_foregroundTask, &Task::requestAbort, this, [this]() -> void {
        _shouldStop = true;

        }, Qt::DirectConnection);

    _shouldStop = false;

    double t = 0.0;
    {
        hdi::utils::ScopedTimer<double> timer(t);

        // Create a context local to this thread that shares with the global share context
        _offscreenBuffer->initialize();

        _foregroundTask->setSubtaskStarted(subtasks.first(), "Computing similarities");
        {
            if (!_hasProbabilityDistribution)
                computeSimilarities();
        }
        _foregroundTask->setSubtaskStarted(subtasks.first(), "Similarities computed");

        QCoreApplication::processEvents();

        computeGradientDescent(_parameters.getNumIterations());
    }
    qDebug() << "t-SNE total compute time: " << t / 1000 << " seconds.";

    if (_shouldStop)
        _foregroundTask->setAborted();
    else
        _foregroundTask->setFinished(QString("t-SNE finished in: %1 seconds").arg(QString::number(t / 1000)));
}

void TsneWorker::continueComputation(int iterations)
{
    QStringList subtasks;

    for (int stepIndex = 0; stepIndex < _parameters.getNumIterations(); stepIndex++)
        subtasks << QString("Step %1").arg(QString::number(stepIndex));

    _foregroundTask->setSubtasks(subtasks);
    _foregroundTask->setRunning();

    connect(_foregroundTask, &Task::requestAbort, this, [this]() -> void {
        _shouldStop = true;

    }, Qt::DirectConnection);

    _shouldStop = false;

    double t = 0.0;
    {
        hdi::utils::ScopedTimer<double> timer(t);
        computeGradientDescent(iterations);
    }

    _foregroundTask->setFinished(QString("t-SNE finished in: %1 seconds").arg(QString::number(t / 1000)));
}

void TsneWorker::stop()
{
    _shouldStop = true;
}

TsneAnalysis::TsneAnalysis() :
    _tsneWorker(nullptr),
    _foregroundTask(this, "TSNE computation")
{
    qRegisterMetaType<TsneData>();

    //_workerThread.start();

    _foregroundTask.setName("TSNE computation");
    _foregroundTask.setMayKill(true);
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

ForegroundTask* TsneAnalysis::getForegroundTask()
{
    return &_foregroundTask;
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
