#include "TsneAnalysis.h"

#include <vector>
#include <assert.h>

#include "TsneAnalysisPlugin.h"
#include <QDebug>

#include "hdi/dimensionality_reduction/tsne_parameters.h"
#include "hdi/utils/scoped_timers.h"

#include <QWindow>
#include <QOpenGLContext>
#include <chrono>
#include <fstream>

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

TsneAnalysis::~TsneAnalysis()
{
}

void TsneAnalysis::computeGradientDescent()
{
    initGradientDescent();

    embed();
}

template <typename sparse_scalar_matrix_type, class output_stream_type>
void saveSparseMatrix(const sparse_scalar_matrix_type& matrix, output_stream_type& stream) {
    typedef float io_scalar_type;
    typedef uint32_t io_unsigned_int_type;

    //number of rows first
    io_unsigned_int_type num_rows = static_cast<io_unsigned_int_type>(matrix.size());
    stream.write(reinterpret_cast<char*>(&num_rows), sizeof(io_unsigned_int_type));
    for (int j = 0; j < num_rows; ++j) {
        //number of elements in the current row
        io_unsigned_int_type num_elems = static_cast<io_unsigned_int_type>(matrix[j].size());

        stream.write(reinterpret_cast<char*>(&num_elems), sizeof(io_unsigned_int_type));
        for (auto& elem : matrix[j]) {
            io_unsigned_int_type id = static_cast<io_unsigned_int_type>(elem.first);
            io_scalar_type v = static_cast<io_scalar_type>(elem.second);
            stream.write(reinterpret_cast<char*>(&id), sizeof(io_unsigned_int_type));
            stream.write(reinterpret_cast<char*>(&v), sizeof(io_scalar_type));
        }
    }
}

template <typename sparse_scalar_matrix_type, class output_stream_type>
void loadSparseMatrix(sparse_scalar_matrix_type& matrix, output_stream_type& stream) {
    typedef float io_scalar_type;
    typedef uint32_t io_unsigned_int_type;

    //number of rows first
    io_unsigned_int_type num_rows;
    stream.read(reinterpret_cast<char*>(&num_rows), sizeof(io_unsigned_int_type));
    matrix.clear();
    matrix.resize(num_rows);
    for (int j = 0; j < num_rows; ++j) {
        //number of elements in the current row
        io_unsigned_int_type num_elems;
        stream.read(reinterpret_cast<char*>(&num_elems), sizeof(io_unsigned_int_type));
        for (int i = 0; i < num_elems; ++i) {
            io_unsigned_int_type id;
            io_scalar_type v;
            stream.read(reinterpret_cast<char*>(&id), sizeof(io_unsigned_int_type));
            stream.read(reinterpret_cast<char*>(&v), sizeof(io_scalar_type));
            matrix[j][id] = v;
        }
    }
}

void TsneAnalysis::initTSNE(const std::vector<float>& data, const int numDimensions)
{
    unsigned int numPoints = data.size() / numDimensions;
    qDebug() << "Variables set. Num dims: " << numDimensions << " Num data points: " << numPoints;
    
    // Input data
    _inputData.assign(numPoints, numDimensions, data);
    qDebug() << "t-SNE input data assigned.";
    
    /// Computation of the high dimensional similarities
    qDebug() << "Output allocated.";
    hdi::dr::HDJointProbabilityGenerator<float>::sparse_scalar_matrix_type distributions;
    std::ifstream input_file("raw_mnist_P", std::ios::binary);
    if (input_file.is_open())
    {
        loadSparseMatrix(_probabilityDistribution, input_file);
    }

    //{
    //    hdi::dr::HDJointProbabilityGenerator<float>::Parameters probGenParams;
    //    probGenParams._perplexity = _perplexity;
    //    probGenParams._perplexity_multiplier = 1;
    //    probGenParams._num_trees = _numTrees;
    //    probGenParams._num_checks = _numChecks;

    //    qDebug() << "tSNE initialized.";

    //    _probabilityDistribution.clear();
    //    _probabilityDistribution.resize(numPoints);
    //    qDebug() << "Sparse matrix allocated.";

    //    qDebug() << "Computing high dimensional probability distributions.. Num dims: " << numDimensions << " Num data points: " << numPoints;
    //    hdi::dr::HDJointProbabilityGenerator<float> probabilityGenerator;
    //    double t = 0.0;
    //    {
    //        hdi::utils::ScopedTimer<double> timer(t);
    //        probabilityGenerator.computeJointProbabilityDistribution(_inputData.getDataNonConst().data(), numDimensions, numPoints, _probabilityDistribution, probGenParams);
    //    }
    //    qDebug() << "Probability distributions calculated.";
    //    qDebug() << "================================================================================";
    //    qDebug() << "A-tSNE: Compute probability distribution: " << t / 1000 << " seconds";
    //    qDebug() << "--------------------------------------------------------------------------------";
    //}
    //std::ofstream stream("probs", std::ios::binary);
    //saveSparseMatrix(_probabilityDistribution, stream);
}

void TsneAnalysis::initGradientDescent()
{
  qDebug() << "Initializing gradient descent";
    _continueFromIteration = 0;

    _isTsneRunning = true;

    hdi::dr::TsneParameters tsneParams;

    tsneParams._embedding_dimensionality = _numDimensionsOutput;
    tsneParams._mom_switching_iter = _exaggerationIter;
    tsneParams._remove_exaggeration_iter = _exaggerationIter;
    tsneParams._exponential_decay_iter = 150;
    tsneParams._exaggeration_factor = 10 + _inputData.getNumPoints() / 5000.;
    _A_tSNE.setTheta(std::min(0.5, std::max(0.0, (_inputData.getNumPoints() - 1000.0)*0.00005)));
    qDebug() << "Initializing2 gradient descent";
    // Create a context local to this thread that shares with the global share context
    offBuffer = new OffscreenBuffer();
    offBuffer->initialize();
    qDebug() << "Initializing3 gradient descent";
    // Initialize GPGPU-SNE
    offBuffer->bindContext();
    qDebug() << "Initializing4 gradient descent";
    _GPGPU_tSNE.initialize(_probabilityDistribution, &_embedding, tsneParams);
    qDebug() << "Initializing5 gradient descent";
    copyFloatOutput();
}

// Computing gradient descent
void TsneAnalysis::embed()
{
    double elapsed = 0;

    {
        qDebug() << "A-tSNE: Computing gradient descent..\n";
        hdi::utils::ScopedTimer<double> timer(elapsed);

        _isGradientDescentRunning = true;

        // Performs gradient descent for every iteration
        for (int iter = 0; iter < _iterations; ++iter)
        {
            
            if (!_isGradientDescentRunning)
            {
                _continueFromIteration = iter;
                break;
            }

            // Perform a GPGPU-SNE iteration
            _GPGPU_tSNE.doAnIteration();

            if (iter > 0 && iter % 2 == 0)
            {
                copyFloatOutput();
                emit newEmbedding();
                //std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
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

void TsneAnalysis::run() {
    computeGradientDescent();
}

// Copy tSNE output to our output
void TsneAnalysis::copyFloatOutput()
{
    _outputData.assign(_inputData.getNumPoints(), _numDimensionsOutput, _embedding.getContainer());
}

const TsneData& TsneAnalysis::output()
{
    return _outputData;
}

void TsneAnalysis::setVerbose(bool verbose)
{
    _verbose = verbose;
}

void TsneAnalysis::setIterations(int iterations)
{
    _iterations = iterations;
}

void TsneAnalysis::setNumTrees(int numTrees)
{
    _numTrees = numTrees;
}

void TsneAnalysis::setNumChecks(int numChecks)
{
    _numChecks = numChecks;
}

void TsneAnalysis::setExaggerationIter(int exaggerationIter)
{
    _exaggerationIter = exaggerationIter;
}

void TsneAnalysis::setPerplexity(int perplexity)
{
    _perplexity = perplexity;
}

void TsneAnalysis::setNumDimensionsOutput(int numDimensionsOutput)
{
    _numDimensionsOutput = numDimensionsOutput;
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
