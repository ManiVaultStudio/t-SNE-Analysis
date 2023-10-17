#pragma once

#include "TsneParameters.h"
#include "TsneData.h"

#include "hdi/dimensionality_reduction/hd_joint_probability_generator.h"
#include "hdi/dimensionality_reduction/gradient_descent_tsne_texture.h"

#include <Task.h>

#include <QThread>

#include <vector>
#include <string>

class OffscreenBuffer;

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
public:
    // The tsne object will compute knn and a probablility distribution before starting the embedding 
    TsneWorker(TsneParameters parameters, /*const*/ std::vector<float>& data, int numDimensions);
    // The tsne object expects a probDist that is not symmetrized, no knn are computed
    TsneWorker(TsneParameters parameters, const std::vector<hdi::data::MapMemEff<uint32_t, float>>& probDist, int numPoints, int numDimensions);
    ~TsneWorker();
    void changeThread(QThread* targetThread);

    int getNumIterations() const;

    void setParentTask(mv::Task* parentTask);
    void createTasks();

public slots:
    void compute();
    void continueComputation(int iterations);
    void stop();

signals:
    void embeddingUpdate(TsneData tsneData);
    void finished();
    void aborted();

private:
    void computeSimilarities();
    void computeGradientDescent(int iterations);
    
    void copyEmbeddingOutput();

    /** Parameters for the execution of the similarity computation and gradient descent */
    TsneParameters _parameters;

    /** Current iteration in the embedding / gradient descent process */
    int _currentIteration;

    // Data variables
    int _numPoints;
    int _numDimensions;

    /** High-dimensional input data */
    std::vector<float> _data;

    /** High-dimensional probability distribution encoding point similarities */
    hdi::dr::HDJointProbabilityGenerator<float>::sparse_scalar_matrix_type _probabilityDistribution;

    /** Check if the worker was initialized with a probability distribution or data */
    bool _hasProbabilityDistribution;

    /** GPGPU t-SNE gradient descent implementation */
    hdi::dr::GradientDescentTSNETexture _GPGPU_tSNE;

    /** Storage of current embedding */
    hdi::data::Embedding<float> _embedding;

    /** Transfer embedding data array */
    TsneData _outEmbedding;

    /** Offscreen OpenGL buffer required to run the gradient descent */
    OffscreenBuffer* _offscreenBuffer;

    // Termination flags
    bool _shouldStop;

private: // Task
    mv::Task*           _parentTask;
    TsneWorkerTasks*    _tasks;
};

class TsneAnalysis : public QObject
{
    Q_OBJECT
public:
    TsneAnalysis();
    ~TsneAnalysis() override;

    void startComputation(TsneParameters parameters, const std::vector<hdi::data::MapMemEff<uint32_t, float>>& probDist, int numPoints, int numDimensions);
    void startComputation(TsneParameters parameters, /*const*/ std::vector<float>& data, int numDimensions);
    void continueComputation(int iterations);
    void stopComputation();
    bool canContinue() const;
    int getNumIterations() const;

    void setTask(mv::Task* task);

private:
    void startComputation(TsneWorker* tsneWorker);

signals:
    // Local signals
    void startWorker();
    void continueWorker(int iterations);
    void stopWorker();

    // Outgoing signals
    void embeddingUpdate(const TsneData tsneData);
    void finished();
    void aborted();

private:
    QThread         _workerThread;
    TsneWorker*     _tsneWorker;
    mv::Task*       _task;
};
