#include "TsneAnalysis.h"

#include <vector>
#include <assert.h>

#include <QDebug>

#include "hdi/dimensionality_reduction/tsne_parameters.h"
#include "hdi/utils/scoped_timers.h"
#include "hdi/utils/glad/glad.h"

#include <QWindow>
#include <QOpenGLContext>

class OffscreenBuffer : public QWindow
{
public:
    OffscreenBuffer()
    {
        setSurfaceType(QWindow::OpenGLSurface);

        create();
    }

    QOpenGLContext* getContext() { return _context; }

    void initialize()
    {
        QOpenGLContext* globalContext = QOpenGLContext::globalShareContext();
        _context = new QOpenGLContext(this);
        _context->setFormat(globalContext->format());

        if (!_context->create())
            qFatal("Cannot create requested OpenGL context.");

        _context->makeCurrent(this);
        if (!gladLoadGL()) {
            qFatal("No OpenGL context is currently bound, therefore OpenGL function loading has failed.");
        }
    }

    void bindContext()
    {
        _context->makeCurrent(this);
    }

    void releaseContext()
    {
        _context->doneCurrent();
    }

private:
    QOpenGLContext* _context;
};

OffscreenBuffer* offBuffer;

TsneAnalysis::TsneAnalysis() :
_verbose(false),
_isGradientDescentRunning(false),
_isTsneRunning(false),
_isMarkedForDeletion(false),
_continueFromIteration(0)
{
    
}

TsneAnalysis::~TsneAnalysis()
{
}

void TsneAnalysis::computeGradientDescent()
{
    initGradientDescent();

    embed();
}

void TsneAnalysis::initTSNE(std::vector<float>& data, const int numDimensions)
{
    emit progressMessage("Initialize A-tSNE");

    unsigned int numPoints = data.size() / numDimensions;
    qDebug() << "Variables set. Num dims: " << numDimensions << " Num data points: " << numPoints;
    
    // Input data
    //_inputData.assign(numPoints, numDimensions, data);
    _numPoints = numPoints;
    _numDimensions = numDimensions;
    qDebug() << "t-SNE input data assigned.";
    
    /// Computation of the high dimensional similarities
    qDebug() << "Output allocated.";
    {
        hdi::dr::HDJointProbabilityGenerator<float>::Parameters probGenParams;
        probGenParams._perplexity = _tsneParameters.getPerplexity();
        probGenParams._perplexity_multiplier = 3;
        probGenParams._num_trees = _tsneParameters.getNumTrees();
        probGenParams._num_checks = _tsneParameters.getNumChecks();
        probGenParams._aknn_algorithm = _tsneParameters.getKnnAlgorithm();
        probGenParams._aknn_metric = _tsneParameters.getKnnDistanceMetric();

        qDebug() << "tSNE initialized.";

        emit progressMessage("tSNE initialized");

        emit progressMessage("Calculate probability distributions");

        _probabilityDistribution.clear();
        _probabilityDistribution.resize(numPoints);
        qDebug() << "Sparse matrix allocated.";

        qDebug() << "Computing high dimensional probability distributions.. Num dims: " << numDimensions << " Num data points: " << numPoints;
        hdi::dr::HDJointProbabilityGenerator<float> probabilityGenerator;
        double t = 0.0;
        {
            hdi::utils::ScopedTimer<double> timer(t);
            probabilityGenerator.computeJointProbabilityDistribution(data.data(), numDimensions, numPoints, _probabilityDistribution, probGenParams);
        }

        emit progressMessage("Probability distributions calculated");

        qDebug() << "Probability distributions calculated.";
        qDebug() << "================================================================================";
        qDebug() << "A-tSNE: Compute probability distribution: " << t / 1000 << " seconds";
        qDebug() << "--------------------------------------------------------------------------------";
    }
}

void TsneAnalysis::initWithProbDist(const int numPoints, const int numDimensions, const std::vector<hdi::data::MapMemEff<uint32_t, float>>& probDist)
{
    unsigned int nump = probDist.size();
    qDebug() << "Variables set. Num dims: " << numDimensions << " Num data points: " << probDist.size();

    _numPoints = nump;
    _numDimensions = numDimensions;
    //_inputData.assign(numPoints, numDimensions, data);
    _probabilityDistribution = probDist;
}

void TsneAnalysis::initGradientDescent()
{
    emit progressMessage("Initializing gradient descent");

    _continueFromIteration = 0;

    _isTsneRunning = true;

    hdi::dr::TsneParameters tsneParams;

    tsneParams._embedding_dimensionality = _tsneParameters.getNumDimensionsOutput();
    tsneParams._mom_switching_iter = _tsneParameters.getExaggerationIter();
    tsneParams._remove_exaggeration_iter = _tsneParameters.getExaggerationIter();
    tsneParams._exponential_decay_iter = _tsneParameters.getExponentialDecayIter();
    tsneParams._exaggeration_factor = 4 + _numPoints / 60000.0;

    // Create a context local to this thread that shares with the global share context
    offBuffer = new OffscreenBuffer();
    offBuffer->initialize();

    // Initialize GPGPU-SNE
    offBuffer->bindContext();
    _GPGPU_tSNE.initialize(_probabilityDistribution, &_embedding, tsneParams);
    
    copyFloatOutput();
}

// Computing gradient descent
void TsneAnalysis::embed()
{
    emit progressMessage("Embedding");

    double elapsed = 0;
    double t = 0;
    {
        qDebug() << "A-tSNE: Computing gradient descent..\n";
        _isGradientDescentRunning = true;

        // Performs gradient descent for every iteration
        for (int iter = 0; iter < _tsneParameters.getNumIterations(); ++iter)
        {
            hdi::utils::ScopedTimer<double> timer(t);
            if (!_isGradientDescentRunning)
            {
                _continueFromIteration = iter;
                break;
            }

            // Perform a GPGPU-SNE iteration
            _GPGPU_tSNE.doAnIteration();

            if (iter > 0 && iter % 10 == 0)
            {
                copyFloatOutput();
                emit newEmbedding();
            }

            if (t > 1000)
                qDebug() << "Time: " << t;

            elapsed += t;

            const auto percentageDone = static_cast<float>(iter) / static_cast<float>(_tsneParameters.getNumIterations());

            emit progressMessage(QString("Computing gradient descent: %1 %").arg(QString::number(100.0f * percentageDone, 'f', 1)));
        }

        offBuffer->releaseContext();

        copyFloatOutput();
        emit newEmbedding();
        
        _isGradientDescentRunning = false;
        _isTsneRunning = false;

        emit computationStopped();
    }

    emit progressMessage("Finished embedding");

    qDebug() << "--------------------------------------------------------------------------------";
    qDebug() << "A-tSNE: Finished embedding of " << "tSNE Analysis" << " in: " << elapsed / 1000 << " seconds ";
    qDebug() << "================================================================================";
}

void TsneAnalysis::run() {
    computeGradientDescent();
}

// Copy tSNE output to our output
void TsneAnalysis::copyFloatOutput()
{
    _outputData.assign(_numPoints, _tsneParameters.getNumDimensionsOutput(), _embedding.getContainer());
}

const TsneData& TsneAnalysis::output()
{
    return _outputData;
}

void TsneAnalysis::setVerbose(bool verbose)
{
    _verbose = verbose;
}

void TsneAnalysis::setParameters(TsneParameters parameters)
{
    _tsneParameters = parameters;
}

void TsneAnalysis::stopGradientDescent()
{
    _isGradientDescentRunning = false;
}

void TsneAnalysis::markForDeletion()
{
    _isMarkedForDeletion = true;

    stopGradientDescent();
}
