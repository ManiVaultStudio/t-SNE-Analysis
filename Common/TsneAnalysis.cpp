#include "TsneAnalysis.h"

#include "OffscreenBuffer.h"

#include <vector>
#include <assert.h>

#include <QDebug>

#include "hdi/dimensionality_reduction/tsne_parameters.h"
#include "hdi/utils/scoped_timers.h"
#include "hdi/utils/glad/glad.h"

TsneWorker::TsneWorker(TsneParameters parameters, const std::vector<hdi::data::MapMemEff<uint32_t, float>>& probDist, int numPoints, int numDimensions) :
    _currentIteration(0),
    _parameters(parameters),
    _probabilityDistribution(probDist),
    _numPoints(numPoints),
    _numDimensions(numDimensions),
    _hasProbabilityDistribution(true),
    _offscreenBuffer(nullptr),
    _shouldStop(false)
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
    _shouldStop(false)
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

void TsneWorker::computeSimilarities()
{
    emit progressSection("Initializing tSNE");

    hdi::dr::HDJointProbabilityGenerator<float>::Parameters probGenParams;
    probGenParams._perplexity = _parameters.getPerplexity();
    probGenParams._perplexity_multiplier = 3;
    probGenParams._num_trees = _parameters.getNumTrees();
    probGenParams._num_checks = _parameters.getNumChecks();
    probGenParams._aknn_algorithm = _parameters.getKnnAlgorithm();
    probGenParams._aknn_metric = _parameters.getKnnDistanceMetric();

    qDebug() << "tSNE initialized.";

    emit progressSection("tSNE initialized");

    emit progressSection("Calculate probability distributions");

    _probabilityDistribution.clear();
    _probabilityDistribution.resize(_numPoints);
    qDebug() << "Sparse matrix allocated.";

    qDebug() << "Computing high dimensional probability distributions.. Num dims: " << _numDimensions << " Num data points: " << _numPoints;
    hdi::dr::HDJointProbabilityGenerator<float> probabilityGenerator;
    double t = 0.0;
    {
        hdi::utils::ScopedTimer<double> timer(t);
        probabilityGenerator.computeJointProbabilityDistribution(_data.data(), _numDimensions, _numPoints, _probabilityDistribution, probGenParams);
    }

    emit progressSection("Probability distributions calculated");
        
    qDebug() << "Probability distributions calculated.";
    qDebug() << "================================================================================";
    qDebug() << "A-tSNE: Compute probability distribution: " << t / 1000 << " seconds";
    qDebug() << "--------------------------------------------------------------------------------";
}

void TsneWorker::computeGradientDescent(int iterations)
{
    if (_shouldStop)
        return;

    emit progressSection("Initializing gradient descent");

    //_currentIteration = 0;

    hdi::dr::TsneParameters tsneParameters;

    tsneParameters._embedding_dimensionality = _parameters.getNumDimensionsOutput();
    tsneParameters._mom_switching_iter = _parameters.getExaggerationIter();
    tsneParameters._remove_exaggeration_iter = _parameters.getExaggerationIter();
    tsneParameters._exponential_decay_iter = _parameters.getExponentialDecayIter();
    tsneParameters._exaggeration_factor = 4 + _numPoints / 60000.0;

    // Initialize GPGPU-SNE
    _offscreenBuffer->bindContext();
    if (_currentIteration == 0)
        _GPGPU_tSNE.initialize(_probabilityDistribution, &_embedding, tsneParameters);

    copyEmbeddingOutput();
    emit embeddingUpdate(_outEmbedding);
    
    const auto emitEmbeddingUpdate = [this](const std::uint32_t& numProcessed, const std::uint32_t& numTotal) -> void {
        emit progressSection(QString("Embedding (step %1 of %2)").arg(QString::number(numProcessed), QString::number(numTotal)));
        //emit progressPercentage(static_cast<float>(numProcessed) / static_cast<float>(numTotal));
    };

    double elapsed = 0;
    double t = 0;
    {
        qDebug() << "A-tSNE: Computing gradient descent..\n";

        const auto beginIteration   = _currentIteration;
        const auto endIteration     = beginIteration + iterations;

        // Performs gradient descent for every iteration
        for (_currentIteration = beginIteration; _currentIteration < endIteration; ++_currentIteration)
        {
            hdi::utils::ScopedTimer<double> timer(t);

            // Perform a GPGPU-SNE iteration
            _GPGPU_tSNE.doAnIteration();

            if (_currentIteration > 0 && _currentIteration % 10 == 0)
            {
                copyEmbeddingOutput();
                emit embeddingUpdate(_outEmbedding);
                emitEmbeddingUpdate(_currentIteration - beginIteration, iterations);
            }

            if (t > 1000)
                qDebug() << "Time: " << t;

            elapsed += t;

            // React to requests to stop
            if (_shouldStop)
                break;
        }

        _offscreenBuffer->releaseContext();

        copyEmbeddingOutput();
        emit embeddingUpdate(_outEmbedding);
    }

    //emit progressSection("Finished embedding");

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
    _shouldStop = false;

    // Create a context local to this thread that shares with the global share context
    _offscreenBuffer->initialize();

    if (!_hasProbabilityDistribution)
        computeSimilarities();
    computeGradientDescent(_parameters.getNumIterations());
}

void TsneWorker::continueComputation(int iterations)
{
    _shouldStop = false;

    computeGradientDescent(iterations);
}

void TsneWorker::stop()
{
    _shouldStop = true;
}

TsneAnalysis::TsneAnalysis() :
    _tsneWorker(nullptr)
{
    qRegisterMetaType<TsneData>();

    //_workerThread.start();
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

    _workerThread.exit();

    // Wait until the thread has terminated (max. 3 seconds)
    if (!_workerThread.wait(3000))
    {
        qDebug() << "tSNE computation thread did not close in time, terminating...";
        _workerThread.terminate();
        _workerThread.wait();
    }
    qDebug() << "tSNE computation stopped.";

    emit stopped();
}

bool TsneAnalysis::canContinue() const
{
    if (_tsneWorker == nullptr)
        return false;

    return _tsneWorker->getNumIterations() >= 1;
}

void TsneAnalysis::startComputation(TsneWorker* tsneWorker)
{
    emit progressPercentage(0.0f);

    tsneWorker->changeThread(&_workerThread);

    // To-Worker signals
    connect(this, &TsneAnalysis::startWorker, tsneWorker, &TsneWorker::compute);
    connect(this, &TsneAnalysis::continueWorker, tsneWorker, &TsneWorker::continueComputation);
    connect(this, &TsneAnalysis::stopWorker, tsneWorker, &TsneWorker::stop);

    // From-Worker signals
    connect(tsneWorker, &TsneWorker::embeddingUpdate, this, &TsneAnalysis::embeddingUpdate);
    connect(tsneWorker, &TsneWorker::progressPercentage, this, &TsneAnalysis::progressPercentage);
    connect(tsneWorker, &TsneWorker::progressSection, this, &TsneAnalysis::progressSection);
    connect(tsneWorker, &TsneWorker::finished, this, &TsneAnalysis::finished);

    // QThread signals
    //connect(&_workerThread, &QThread::finished, tsneWorker, &QObject::deleteLater);

    _workerThread.start();

    emit startWorker();
}
