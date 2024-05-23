#pragma once

#include "KnnParameters.h"
#include "TsneData.h"
#include "TsneParameters.h"

#include "hdi/dimensionality_reduction/gradient_descent_tsne_texture.h"
#include "hdi/dimensionality_reduction/hd_joint_probability_generator.h"
#include "hdi/dimensionality_reduction/sparse_tsne_user_def_probabilities.h"
#include "hdi/dimensionality_reduction/tsne_parameters.h"
#include "hdi/utils/cout_log.h"

#include <Task.h>

#include <QThread>

#include <optional>
#include <vector>

class OffscreenBuffer;

using ProbDistMatrix      = std::vector<hdi::data::MapMemEff<std::uint32_t, float>>;
using ProbDistGenerator   = hdi::dr::HDJointProbabilityGenerator<float, ProbDistMatrix>;
using GradienDescentGPU   = hdi::dr::GradientDescentTSNETexture<float, ProbDistMatrix>;
using GradienDescentCPU   = hdi::dr::SparseTSNEUserDefProbabilities<float, ProbDistMatrix>;

using ProbDistMatrix64      = std::vector<hdi::data::MapMemEff<std::uint64_t, float>>;
using ProbDistGenerator64   = hdi::dr::HDJointProbabilityGenerator<float, ProbDistMatrix64, std::uint64_t, std::int64_t>;
using GradienDescentGPU64   = hdi::dr::GradientDescentTSNETexture<float, ProbDistMatrix64, std::uint64_t, std::int64_t>;
using GradienDescentCPU64   = hdi::dr::SparseTSNEUserDefProbabilities<float, ProbDistMatrix64, std::uint64_t, std::int64_t>;

class TsneWorkerTasks : public QObject
{
public:
    TsneWorkerTasks(QObject* parent, mv::Task* parentTask);

    mv::Task& getInitializeOffScreenBufferTask() { return _initializeOffScreenBufferTask; };
    mv::Task& getComputingSimilaritiesTask() { return _computingSimilaritiesTask; };
    mv::Task& getInitializeTsneTask() { return _initializeTsneTask; };
    mv::Task& getComputeGradientDescentTask() { return _computeGradientDescentTask; };

private:
    mv::Task    _initializeOffScreenBufferTask;
    mv::Task    _computingSimilaritiesTask;
    mv::Task    _initializeTsneTask;
    mv::Task    _computeGradientDescentTask;
};

class TsneWorker : public QObject
{
    Q_OBJECT

private:
    // default construction is inaccessible to outsiders
    TsneWorker(TsneParameters tsneParameters);
public:
    // The tsne object will compute knn and a probablility distribution before starting the embedding 
    TsneWorker(TsneParameters tsneParameters, KnnParameters knnParameters, const std::vector<float>& data, uint32_t numDimensions, const hdi::data::Embedding<float>::scalar_vector_type* initEmbedding);
    // The tsne object will compute knn and a probablility distribution before starting the embedding, moving the input data
    TsneWorker(TsneParameters tsneParameters, KnnParameters knnParameters, std::vector<float>&& data, uint32_t numDimensions, const hdi::data::Embedding<float>::scalar_vector_type* initEmbedding);
    // The tsne object expects a probDist that is not symmetrized, no knn are computed
    TsneWorker(TsneParameters tsneParameters, const ProbDistMatrix& probDist, uint32_t numPoints, const hdi::data::Embedding<float>::scalar_vector_type* initEmbedding);
    // The tsne object expects a probDist that is not symmetrized, no knn are computed, moving the probDist
    TsneWorker(TsneParameters tsneParameters, ProbDistMatrix&& probDist, uint32_t numPoints, const hdi::data::Embedding<float>::scalar_vector_type* initEmbedding);
    // The tsne object expects a probDist that is not symmetrized, no knn are computed
    TsneWorker(TsneParameters tsneParameters, const ProbDistMatrix64& probDist64, uint32_t numPoints, const hdi::data::Embedding<float>::scalar_vector_type* initEmbedding);
    // The tsne object expects a probDist that is not symmetrized, no knn are computed, moving the probDist
    TsneWorker(TsneParameters tsneParameters, ProbDistMatrix64&& probDist64, uint32_t numPoints, const hdi::data::Embedding<float>::scalar_vector_type* initEmbedding);
    ~TsneWorker();

    void createTasks();

public: // Setter
    void setParentTask(mv::Task* parentTask);
    void setInitEmbedding(const hdi::data::Embedding<float>::scalar_vector_type& initEmbedding);
    void setCurrentIteration(int currentIteration);
    void changeThread(QThread* targetThread);

public: // Getter
    ProbDistMatrix* getProbabilityDistribution() { return &_probabilityDistribution; };
    int getNumIterations() const;

public slots:
    void compute();
    void continueComputation(uint32_t iterations);
    void stop();

signals:
    void embeddingUpdate(TsneData tsneData);
    void finished();
    void aborted();

private:
    void computeSimilarities();
    void computeGradientDescent(uint32_t iterations);
    
    void copyEmbeddingOutput();

    hdi::dr::TsneParameters tsneParameters();
    ProbDistGenerator::Parameters probGenParameters();
    ProbDistGenerator64::Parameters probGenParameters64();

    void check64bit();
    void resetThread();

private:
    TsneParameters                          _tsneParameters;                /** Parameters for the execution of the similarity computation and gradient descent */
    KnnParameters                           _knnParameters;                 /** Parameters for the aknn search */
    int                                     _currentIteration;              /** Current iteration in the embedding / gradient descent process */
    uint32_t                                _numPoints;                     /** Data variable */
    uint32_t                                _numDimensions;                 /** Data variable */
    std::vector<float>                      _data;                          /** High-dimensional input data */
    ProbDistMatrix                          _probabilityDistribution;       /** High-dimensional probability distribution encoding point similarities */
    ProbDistMatrix64                        _probabilityDistribution64;     /** High-dimensional probability distribution encoding point similarities, 64 bit */
    bool                                    _hasProbabilityDistribution;    /** Check if the worker was initialized with a probability distribution or data */
    GradienDescentGPU                       _GPGPU_tSNE;                    /** GPGPU t-SNE gradient descent implementation, 32 bit */
    GradienDescentGPU64                     _GPGPU_tSNE64;                  /** GPGPU t-SNE gradient descent implementation, 64 bit */
    GradienDescentCPU                       _CPU_tSNE;                      /** CPU t-SNE gradient descent implementation, 32 bit */
    GradienDescentCPU64                     _CPU_tSNE64;                    /** CPU t-SNE gradient descent implementation, 64 bit */
    hdi::data::Embedding<float>             _embedding;                     /** Storage of current embedding */
    TsneData                                _outEmbedding;                  /** Transfer embedding data array */
    hdi::utils::CoutLog                     _logger;                        /** HDILib logger class */
    OffscreenBuffer*                        _offscreenBuffer;               /** Offscreen OpenGL buffer required to run the gradient descent */
    bool                                    _use64BitImplementation;        /** Wheter to use 64 bit implementation for large data (cannot make use of compute shader) */
    bool                                    _shouldStop;                    /** Termination flags */

private: 
    mv::Task*                               _parentTask;                    /** Task: parent */
    TsneWorkerTasks*                        _tasks;                         /** Task: worker */
};

class TsneAnalysis : public QObject
{
    Q_OBJECT
public:
    TsneAnalysis();
    ~TsneAnalysis() override;

public: // Interactions
    
    // Compute embedding based on pre-computed similarites
    void startComputation(TsneParameters parameters, const ProbDistMatrix& probDist, uint32_t numPoints, const hdi::data::Embedding<float>::scalar_vector_type* initEmbedding = nullptr, int iterations = -1);
    // Compute embedding based on pre-computed similarites, moves the input probDist
    void startComputation(TsneParameters parameters, ProbDistMatrix&& probDist, uint32_t numPoints, const hdi::data::Embedding<float>::scalar_vector_type* initEmbedding = nullptr, int iterations = -1);
    // Compute similarities (aknn search) and embedding
    void startComputation(TsneParameters parameters, KnnParameters knnParameters, const std::vector<float>& data, uint32_t numDimensions, const hdi::data::Embedding<float>::scalar_vector_type* initEmbedding = nullptr);
    // Compute similarities (aknn search) and embedding, moves the input data
    void startComputation(TsneParameters parameters, KnnParameters knnParameters, std::vector<float>&& data, uint32_t numDimensions, const hdi::data::Embedding<float>::scalar_vector_type* initEmbedding = nullptr);
    
    void continueComputation(int previousIterations);
    void stopComputation();

public: // Setter
    void setTask(mv::Task* task);
    void setInitEmbedding(const hdi::data::Embedding<float>::scalar_vector_type& initEmbedding);

public: // Getter
    int getNumIterations() const { return (_tsneWorker) ? _tsneWorker->getNumIterations() : -1; };
    bool canContinue() const { return (_tsneWorker) ? _tsneWorker->getNumIterations() >= 1 : false; };
    std::optional<ProbDistMatrix*> getProbabilityDistribution() { return (_tsneWorker) ? std::optional<ProbDistMatrix*>(_tsneWorker->getProbabilityDistribution()) : std::nullopt; };
    const std::optional<ProbDistMatrix*> getProbabilityDistribution() const { return (_tsneWorker) ? std::optional<ProbDistMatrix*>(_tsneWorker->getProbabilityDistribution()) : std::nullopt; };

private: // Internal
    void startComputation(TsneWorker* tsneWorker);
    void deleteWorker();

signals:
    // Local signals
    void startWorker();
    void continueWorker(uint32_t iterations);
    void stopWorker();

    // Outgoing signals
    void embeddingUpdate(const TsneData tsneData);
    void started();
    void finished();
    void aborted();

private:
    QThread         _workerThread;
    TsneWorker*     _tsneWorker;
    mv::Task*       _task;
};
