#include "TsneAnalysis.h"

#include "hdi/utils/glad/glad.h"
#include "OffscreenBuffer.h"

#include <cassert>
#include <vector>

#include <QCoreApplication>
#include <QDebug>

#include "hdi/utils/scoped_timers.h"

using namespace mv;

TsneWorker::TsneWorker(TsneParameters tsneParameters) :
    _currentIteration(0),
    _tsneParameters(tsneParameters),
    _knnParameters(),
    _numPoints(0),
    _numDimensions(0),
    _data(),
    _probabilityDistribution(),
    _hasProbabilityDistribution(false),
    _GPGPU_tSNE(),
    _CPU_tSNE(),
    _embedding(),
    _outEmbedding(),
    _offscreenBuffer(nullptr),
    _shouldStop(false),
    _parentTask(nullptr),
    _tasks(nullptr)
{
    // Offscreen buffer must be created in the UI thread because it is a QWindow, afterwards we move it
    _offscreenBuffer = new OffscreenBuffer();
}

TsneWorker::TsneWorker(TsneParameters tsneParameters, KnnParameters knnParameters, const std::vector<float>& data, uint32_t numDimensions, const hdi::data::Embedding<float>::scalar_vector_type* initEmbedding) :
    TsneWorker(tsneParameters)
{
    _knnParameters = knnParameters;
    assert(numDimensions > 0);
    _numPoints = data.size() / numDimensions;
    _numDimensions = numDimensions;
    _data = data;
    _embedding = { static_cast<uint32_t>(_tsneParameters.getNumDimensionsOutput()), _numPoints };

    if (initEmbedding)
        setInitEmbedding(*initEmbedding);
}

TsneWorker::TsneWorker(TsneParameters parameters, KnnParameters knnParameters, std::vector<float>&& data, uint32_t numDimensions, const hdi::data::Embedding<float>::scalar_vector_type* initEmbedding) :
    TsneWorker(parameters)
{
    _knnParameters = knnParameters;
    assert(numDimensions > 0);
    _numPoints = data.size() / numDimensions;
    _numDimensions = numDimensions;
    _data = std::move(data);
    _embedding = { static_cast<uint32_t>(_tsneParameters.getNumDimensionsOutput()), _numPoints };

    if (initEmbedding)
        setInitEmbedding(*initEmbedding);
}

TsneWorker::TsneWorker(TsneParameters parameters, const std::vector<hdi::data::MapMemEff<uint32_t, float>>& probDist, uint32_t numPoints, const hdi::data::Embedding<float>::scalar_vector_type* initEmbedding) :
    TsneWorker(parameters)
{
    _probabilityDistribution = probDist;
    _hasProbabilityDistribution = true;
    _numPoints = numPoints;
    _embedding = { static_cast<uint32_t>(_tsneParameters.getNumDimensionsOutput()), _numPoints };

    if (initEmbedding)
        setInitEmbedding(*initEmbedding);
}

TsneWorker::TsneWorker(TsneParameters parameters, std::vector<hdi::data::MapMemEff<uint32_t, float>>&& probDist, uint32_t numPoints, const hdi::data::Embedding<float>::scalar_vector_type* initEmbedding) :
    TsneWorker(parameters)
{
    _probabilityDistribution = std::move(probDist);
    _hasProbabilityDistribution = true;
    _numPoints = numPoints;
    _embedding = { static_cast<uint32_t>(_tsneParameters.getNumDimensionsOutput()), _numPoints };
    _tsneParameters.setExaggerationFactor(4 + _numPoints / 60000.0);

    if (initEmbedding)
        setInitEmbedding(*initEmbedding);
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
    _offscreenBuffer->getContext()->moveToThread(targetThread);
}

void TsneWorker::resetThread()
{
    changeThread(QCoreApplication::instance()->thread());
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

void TsneWorker::setInitEmbedding(const hdi::data::Embedding<float>::scalar_vector_type& initEmbedding)
{
    assert(initEmbedding.size() == _embedding.numDataPoints() * _embedding.numDimensions());
    _embedding.getContainer() = initEmbedding;
    _tsneParameters.setPresetEmbedding(true);
}

void TsneWorker::setCurrentIteration(int currentIteration)
{
    if(currentIteration < 0)
        return;
    
    _currentIteration = currentIteration;

    // we do not want to reapeat the exageration phase when continuing the gradient descent
    if (_currentIteration > (_tsneParameters.getExaggerationIter() + _tsneParameters.getExponentialDecayIter()))
    {
        _tsneParameters.setExaggerationFactor(1);
        _tsneParameters.setExaggerationIter(0);
        _tsneParameters.setExponentialDecayIter(0);
    }
    else if (_currentIteration > _tsneParameters.getExaggerationIter())
    {
        double decay = 1. - double(_currentIteration - _tsneParameters.getExaggerationIter()) / _tsneParameters.getExponentialDecayIter();
        _tsneParameters.setExaggerationFactor( 1 + (_tsneParameters.getExaggerationFactor() - 1) * decay );
    }
}

void TsneWorker::createTasks()
{
    _tasks = new TsneWorkerTasks(this, _parentTask);
}

hdi::dr::TsneParameters TsneWorker::tsneParameters()
{
    hdi::dr::TsneParameters tsneParameters;

    tsneParameters._embedding_dimensionality    = _tsneParameters.getNumDimensionsOutput();
    tsneParameters._mom_switching_iter          = _tsneParameters.getExaggerationIter();
    tsneParameters._remove_exaggeration_iter    = _tsneParameters.getExaggerationIter();
    tsneParameters._exaggeration_factor         = _tsneParameters.getExaggerationFactor();
    tsneParameters._exponential_decay_iter      = _tsneParameters.getExponentialDecayIter();
    tsneParameters._presetEmbedding             = _tsneParameters.getPresetEmbedding();

    return tsneParameters;
}

hdi::dr::HDJointProbabilityGenerator<float>::Parameters TsneWorker::probGenParameters()
{
    hdi::dr::HDJointProbabilityGenerator<float>::Parameters probGenParams;

    probGenParams._perplexity               = _tsneParameters.getPerplexity();
    probGenParams._perplexity_multiplier    = 3;
    probGenParams._num_trees                = _knnParameters.getAnnoyNumTrees();
    probGenParams._num_checks               = _knnParameters.getAnnoyNumChecks();
    probGenParams._aknn_algorithmP1         = _knnParameters.getHNSWm();
    probGenParams._aknn_algorithmP2         = _knnParameters.getHNSWef();
    probGenParams._aknn_algorithm           = _knnParameters.getKnnAlgorithm();
    probGenParams._aknn_metric              = _knnParameters.getKnnDistanceMetric();

    return probGenParams;
}

void TsneWorker::computeSimilarities()
{
    assert(_data.size() == _numDimensions * _numPoints);

    _tasks->getComputingSimilaritiesTask().setRunning();

    double t = 0.0;
    {
        hdi::utils::ScopedTimer<double> timer(t);

        _probabilityDistribution.clear();
        _probabilityDistribution.resize(_numPoints);
        qDebug() << "Sparse matrix allocated.";

        hdi::dr::HDJointProbabilityGenerator<float> probabilityGenerator;

        qDebug() << "Computing high dimensional probability distributions: Num dims: " << _numDimensions << " Num data points: " << _numPoints;
        probabilityGenerator.computeJointProbabilityDistribution(_data.data(), _numDimensions, _numPoints, _probabilityDistribution, probGenParameters());         // The _probabilityDistribution is symmetrized here.
    }
    
    qDebug() << "================================================================================";
    qDebug() << "tSNE: Computed probability distribution: " << t / 1000 << " seconds";
    qDebug() << "--------------------------------------------------------------------------------";

    _tasks->getComputingSimilaritiesTask().setFinished();
}

void TsneWorker::computeGradientDescent(uint32_t iterations)
{
    if (_shouldStop)
        return;

    const auto updateEmbedding = [this](const TsneData& tsneData) -> void {
        copyEmbeddingOutput();
        emit embeddingUpdate(tsneData);
        };

    auto initGPUTSNE = [this]() {
        // Initialize offscreen buffer
        double t_buffer = 0.0;
        {
            hdi::utils::ScopedTimer<double> timer(t_buffer);
            _offscreenBuffer->bindContext();
        }
        qDebug() << "tSNE: Set up offscreen buffer in " << t_buffer / 1000 << " seconds.";

        if (!_GPGPU_tSNE.isInitialized())
        {
            auto params = tsneParameters();

            // In case of HSNE, the _probabilityDistribution is a non-summetric transition matrix and initialize() symmetrizes it here
            if (_hasProbabilityDistribution)
                _GPGPU_tSNE.initialize(_probabilityDistribution, &_embedding, params);
            else
                _GPGPU_tSNE.initializeWithJointProbabilityDistribution(_probabilityDistribution, &_embedding, params);

            qDebug() << "A-tSNE (GPU): Exaggeration factor: " << params._exaggeration_factor << ", exaggeration iterations: " << params._remove_exaggeration_iter << ", exaggeration decay iter: " << params._exponential_decay_iter;
        }
    };

    auto initCPUTSNE = [this]() {
        if (!_CPU_tSNE.isInitialized())
        {
            auto params = tsneParameters();

            double theta = std::min(0.5, std::max(0.0, (_numPoints - 1000.0) * 0.00005));
            _CPU_tSNE.setTheta(theta);

            // In case of HSNE, the _probabilityDistribution is a non-summetric transition matrix and initialize() symmetrizes it here
            if (_hasProbabilityDistribution)
                _CPU_tSNE.initialize(_probabilityDistribution, &_embedding, params);
            else
                _CPU_tSNE.initializeWithJointProbabilityDistribution(_probabilityDistribution, &_embedding, params);

            qDebug() << "t-SNE (CPU, Barnes-Hut): Exaggeration factor: " << params._exaggeration_factor << ", exaggeration iterations: " << params._remove_exaggeration_iter << ", exaggeration decay iter: " << params._exponential_decay_iter << ", theta: " << theta;
        }
    };

    auto initTSNE = [this, initGPUTSNE, initCPUTSNE, updateEmbedding]() {
        double t_init = 0.0;
        {
            hdi::utils::ScopedTimer<double> timer(t_init);

            if (_tsneParameters.getGradientDescentType() == GradientDescentType::GPU)
                initGPUTSNE();
            else
                initCPUTSNE();

            updateEmbedding(_outEmbedding);
        }
        qDebug() << "tSNE: Init t-SNE " << t_init / 1000 << " seconds.";
    };

    auto singleTSNEIteration = [this]() {
        if (_tsneParameters.getGradientDescentType() == GradientDescentType::GPU)
            _GPGPU_tSNE.doAnIteration();
        else
            _CPU_tSNE.doAnIteration();
    };

    auto gradientDescentCleanup = [this]() {
        if (_tsneParameters.getGradientDescentType() == GradientDescentType::GPU)
            _offscreenBuffer->releaseContext();
        else
            return; // Nothing to do for CPU implementation
    };

    _tasks->getInitializeTsneTask().setRunning();

    initTSNE();

    _tasks->getInitializeTsneTask().setFinished();

    const auto beginIteration = _currentIteration;
    const auto endIteration = beginIteration + iterations;

    double elapsed = 0;
    double t_grad = 0;
    {
        qDebug() << "tSNE: Computing " << endIteration - beginIteration << " gradient descent iterations...";

        _tasks->getComputeGradientDescentTask().setRunning();
        _tasks->getComputeGradientDescentTask().setSubtasks(iterations);

        int currentStepIndex = 0;

        // Performs gradient descent for every iteration
        for (_currentIteration = beginIteration; _currentIteration < endIteration; ++_currentIteration) {

            _tasks->getComputeGradientDescentTask().setSubtaskStarted(currentStepIndex);

            hdi::utils::ScopedTimer<double> timer(t_grad);

            // Perform t-SNE iteration
            singleTSNEIteration();

            if (_currentIteration > 0 && _tsneParameters.getUpdateCore() > 0 && _currentIteration % _tsneParameters.getUpdateCore() == 0)
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

        gradientDescentCleanup();

        updateEmbedding(_outEmbedding);

        _tasks->getComputeGradientDescentTask().setFinished();
    }

    qDebug() << "--------------------------------------------------------------------------------";
    qDebug() << "tSNE: Finished embedding in: " << elapsed / 1000 << " seconds, with " << _currentIteration << " total iterations (" << endIteration - beginIteration << " new iterations)";
    qDebug() << "================================================================================";

    emit finished();
}

void TsneWorker::copyEmbeddingOutput()
{
    _outEmbedding.assign(_numPoints, _tsneParameters.getNumDimensionsOutput(), _embedding.getContainer());
}

void TsneWorker::compute()
{
    createTasks();

    connect(_parentTask, &Task::requestAbort, this, [this]() -> void { _shouldStop = true; }, Qt::DirectConnection);

    _shouldStop = false;

    double t = 0.0;
    {
        hdi::utils::ScopedTimer<double> timer(t);

        if (!_hasProbabilityDistribution)
            computeSimilarities();

        computeGradientDescent(_tsneParameters.getNumIterations());
    }
 
    qDebug() << "t-SNE total compute time: " << t / 1000 << " seconds.";

    if (_shouldStop)
        _tasks->getComputeGradientDescentTask().setAborted();
    else
        _tasks->getComputeGradientDescentTask().setFinished();

    _parentTask->setFinished();

    resetThread();
}

void TsneWorker::continueComputation(uint32_t iterations)
{
    _tasks->getInitializeOffScreenBufferTask().setEnabled(false);
    _tasks->getComputingSimilaritiesTask().setEnabled(false);
    _tasks->getInitializeTsneTask().setEnabled(false);
    
    connect(_parentTask, &Task::requestAbort, this, [this]() -> void { _shouldStop = true; }, Qt::DirectConnection);

    _shouldStop = false;

    computeGradientDescent(iterations);

    _parentTask->setFinished();

    resetThread();
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
    _workerThread.quit();           // Signal the thread to quit gracefully
    if (!_workerThread.wait(500))   // Wait for the thread to actually finish
        _workerThread.terminate();  // Terminate thread after 0.5 seconds

    deleteWorker();
}

void TsneAnalysis::deleteWorker()
{
    if (_tsneWorker)
    {
        _tsneWorker->changeThread(QThread::currentThread());
        delete _tsneWorker;
    }
}

void TsneAnalysis::startComputation(TsneParameters parameters, const std::vector<hdi::data::MapMemEff<uint32_t, float>>& probDist, uint32_t numPoints, const hdi::data::Embedding<float>::scalar_vector_type* initEmbedding, int previousIterations)
{
    deleteWorker();

    _tsneWorker = new TsneWorker(parameters, probDist, numPoints, initEmbedding);

    if (previousIterations >= 0)
        _tsneWorker->setCurrentIteration(previousIterations);

    startComputation();
}

void TsneAnalysis::startComputation(TsneParameters parameters, std::vector<hdi::data::MapMemEff<uint32_t, float>>&& probDist, uint32_t numPoints, const hdi::data::Embedding<float>::scalar_vector_type* initEmbedding, int previousIterations)
{
    deleteWorker();

    _tsneWorker = new TsneWorker(parameters, std::move(probDist), numPoints, initEmbedding);

    if (previousIterations >= 0)
        _tsneWorker->setCurrentIteration(previousIterations);

    startComputation();
}

void TsneAnalysis::startComputation(TsneParameters parameters, KnnParameters knnParameters, const std::vector<float>& data, uint32_t numDimensions, const hdi::data::Embedding<float>::scalar_vector_type* initEmbedding)
{
    deleteWorker();

    _tsneWorker = new TsneWorker(parameters, knnParameters, data, numDimensions, initEmbedding);
    
    startComputation();
}

void TsneAnalysis::startComputation(TsneParameters parameters, KnnParameters knnParameters, std::vector<float>&& data, uint32_t numDimensions, const hdi::data::Embedding<float>::scalar_vector_type* initEmbedding)
{
    deleteWorker();

    _tsneWorker = new TsneWorker(parameters, knnParameters, std::move(data), numDimensions, initEmbedding);
    
    startComputation();
}

void TsneAnalysis::continueComputation(int iterations)
{
    if (!canContinue())
        return;

    _tsneWorker->changeThread(&_workerThread);

    emit continueWorker(iterations);
}

void TsneAnalysis::stopComputation()
{
    emit stopWorker();  // to _workerThread in Thread
    
    emit aborted();     // to external listeners
}

void TsneAnalysis::setTask(mv::Task* task)
{
    assert(task);
    _task = task;
}

void TsneAnalysis::setInitEmbedding(const hdi::data::Embedding<float>::scalar_vector_type& initEmbedding)
{
    if (_tsneWorker)
        _tsneWorker->setInitEmbedding(initEmbedding);
}

void TsneAnalysis::startComputation()
{
    _tsneWorker->setParentTask(_task);

    _tsneWorker->getOffscreenBuffer().initialize();
    _tsneWorker->changeThread(&_workerThread);
    
    // To-Worker signals
    connect(this, &TsneAnalysis::startWorker, _tsneWorker, &TsneWorker::compute);
    connect(this, &TsneAnalysis::continueWorker, _tsneWorker, &TsneWorker::continueComputation);
    connect(this, &TsneAnalysis::stopWorker, _tsneWorker, &TsneWorker::stop, Qt::DirectConnection);

    // From-Worker signals
    connect(_tsneWorker, &TsneWorker::embeddingUpdate, this, &TsneAnalysis::embeddingUpdate);
    connect(_tsneWorker, &TsneWorker::finished, this, &TsneAnalysis::finished);

    _workerThread.start();

    emit startWorker();
    emit started();
}

TsneWorkerTasks::TsneWorkerTasks(QObject* parent, mv::Task* parentTask) :
    QObject(parent),
    _initializeOffScreenBufferTask(this, "Initialize off-screen GPGPU buffer", Task::GuiScopes{ Task::GuiScope::DataHierarchy }, Task::Status::Idle),
    _computingSimilaritiesTask(this, "Compute similarities", Task::GuiScopes{ Task::GuiScope::DataHierarchy }, Task::Status::Idle),
    _initializeTsneTask(this, "Initialize TSNE", Task::GuiScopes{ Task::GuiScope::DataHierarchy }, Task::Status::Idle),
    _computeGradientDescentTask(this, "Compute gradient descent", Task::GuiScopes{ Task::GuiScope::DataHierarchy }, Task::Status::Idle)
{
    _initializeOffScreenBufferTask.setParentTask(parentTask);
    _computingSimilaritiesTask.setParentTask(parentTask);
    _initializeTsneTask.setParentTask(parentTask);
    _computeGradientDescentTask.setParentTask(parentTask);

    _computeGradientDescentTask.setWeight(20.f);
    _computeGradientDescentTask.setSubtaskNamePrefix("Compute gradient descent step");

    /*
    _initializeOffScreenBufferTask.moveToThread(targetThread);
    _computingSimilaritiesTask.moveToThread(targetThread);
    _initializeTsneTask.moveToThread(targetThread);
    _computeGradientDescentTask.moveToThread(targetThread);
    */
}
