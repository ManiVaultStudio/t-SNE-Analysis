#include "HsneAnalysis.h"

#include <vector>
#include <assert.h>

#include "TsneAnalysisPlugin.h"
#include <QDebug>

#include "hdi/dimensionality_reduction/tsne_parameters.h"
#include "hdi/utils/scoped_timers.h"

#include <QWindow>
#include <QOpenGLContext>

namespace
{
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
}

OffscreenBuffer* offBuffer;

HsneAnalysis::HsneAnalysis() :
_iterations(1000),
_numTrees(4),
_numChecks(1024),
_exaggerationIter(250),
_perplexity(30),
_numDimensionsOutput(2),
_verbose(false),
_isGradientDescentRunning(false),
_isTsneRunning(false),
_isMarkedForDeletion(false),
_continueFromIteration(0)
{
    
}

HsneAnalysis::~HsneAnalysis()
{
}

void HsneAnalysis::computeGradientDescent()
{
    initGradientDescent();

    embed();
}

void HsneAnalysis::initTSNE(const std::vector<float>& data, const int numDimensions)
{
    unsigned int numPoints = data.size() / numDimensions;
    qDebug() << "Variables set. Num dims: " << numDimensions << " Num data points: " << numPoints;
    
    // Input data
    _inputData.assign(numPoints, numDimensions, data);
    qDebug() << "t-SNE input data assigned.";
    
    /// Computation of the high dimensional similarities
    qDebug() << "Output allocated.";
    {
        hdi::dr::HDJointProbabilityGenerator<float>::Parameters probGenParams;
        probGenParams._perplexity = _perplexity;
        probGenParams._perplexity_multiplier = 3;
        probGenParams._num_trees = _numTrees;
        probGenParams._num_checks = _numChecks;

        qDebug() << "tSNE initialized.";

        _probabilityDistribution.clear();
        _probabilityDistribution.resize(numPoints);
        qDebug() << "Sparse matrix allocated.";

        qDebug() << "Computing high dimensional probability distributions.. Num dims: " << numDimensions << " Num data points: " << numPoints;
        hdi::dr::HDJointProbabilityGenerator<float> probabilityGenerator;
        double t = 0.0;
        {
            hdi::utils::ScopedTimer<double> timer(t);
            probabilityGenerator.computeJointProbabilityDistribution(_inputData.getDataNonConst().data(), numDimensions, numPoints, _probabilityDistribution, probGenParams);
        }
        qDebug() << "Probability distributions calculated.";
        qDebug() << "================================================================================";
        qDebug() << "A-tSNE: Compute probability distribution: " << t / 1000 << " seconds";
        qDebug() << "--------------------------------------------------------------------------------";
    }
}

void HsneAnalysis::initGradientDescent()
{
    _continueFromIteration = 0;

    _isTsneRunning = true;

    hdi::dr::TsneParameters tsneParams;

    tsneParams._embedding_dimensionality = _numDimensionsOutput;
    tsneParams._mom_switching_iter = _exaggerationIter;
    tsneParams._remove_exaggeration_iter = _exaggerationIter;
    tsneParams._exponential_decay_iter = 150;
    tsneParams._exaggeration_factor = 4 + _inputData.getNumPoints() / 60000.0;
//    _A_tSNE.setTheta(std::min(0.5, std::max(0.0, (_inputData.getNumPoints() - 1000.0)*0.00005)));

    // Create a context local to this thread that shares with the global share context
    offBuffer = new OffscreenBuffer();
    offBuffer->initialize();

    // Initialize GPGPU-SNE
    offBuffer->bindContext();
    _GPGPU_tSNE.initialize(_probabilityDistribution, &_embedding, tsneParams);
    
    copyFloatOutput();
}

// Computing gradient descent
void HsneAnalysis::embed()
{
    double elapsed = 0;
    double t = 0;
    {
        qDebug() << "A-tSNE: Computing gradient descent..\n";
        _isGradientDescentRunning = true;

        // Performs gradient descent for every iteration
        for (int iter = 0; iter < _iterations; ++iter)
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
        }
        offBuffer->releaseContext();

        copyFloatOutput();
        emit newEmbedding();
        
        _isGradientDescentRunning = false;
        _isTsneRunning = false;

        emit computationStopped();
    }

    qDebug() << "--------------------------------------------------------------------------------";
    qDebug() << "A-tSNE: Finished embedding of " << "tSNE Analysis" << " in: " << elapsed / 1000 << " seconds ";
    qDebug() << "================================================================================";
}

void HsneAnalysis::run() {
    computeGradientDescent();
}

// Copy tSNE output to our output
void HsneAnalysis::copyFloatOutput()
{
    _outputData.assign(_inputData.getNumPoints(), _numDimensionsOutput, _embedding.getContainer());
}

const TsneData& HsneAnalysis::output()
{
    return _outputData;
}

void HsneAnalysis::setVerbose(bool verbose)
{
    _verbose = verbose;
}

void HsneAnalysis::setIterations(int iterations)
{
    _iterations = iterations;
}

void HsneAnalysis::setNumTrees(int numTrees)
{
    _numTrees = numTrees;
}

void HsneAnalysis::setNumChecks(int numChecks)
{
    _numChecks = numChecks;
}

void HsneAnalysis::setExaggerationIter(int exaggerationIter)
{
    _exaggerationIter = exaggerationIter;
}

void HsneAnalysis::setPerplexity(int perplexity)
{
    _perplexity = perplexity;
}

void HsneAnalysis::setNumDimensionsOutput(int numDimensionsOutput)
{
    _numDimensionsOutput = numDimensionsOutput;
}

void HsneAnalysis::stopGradientDescent()
{
    _isGradientDescentRunning = false;
}

void HsneAnalysis::markForDeletion()
{
    _isMarkedForDeletion = true;

    stopGradientDescent();
}
