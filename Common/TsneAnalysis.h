#pragma once

#include "TsneData.h"

#include "hdi/dimensionality_reduction/hd_joint_probability_generator.h"
#include "hdi/dimensionality_reduction/sparse_tsne_user_def_probabilities.h"
#include "hdi/dimensionality_reduction/gradient_descent_tsne_texture.h"

#include <QThread>

#include <vector>
#include <string>

class TsneAnalysis : public QThread
{
    Q_OBJECT
public:
    TsneAnalysis();
    ~TsneAnalysis() override;

    void setKnnAlgorithm(int algorithm);
    void setDistanceMetric(int metric);
    void setVerbose(bool verbose);
    void setIterations(int iterations);
    void setNumTrees(int numTrees);
    void setNumChecks(int numChecks);
    void setExaggerationIter(int exaggerationIter);
    void setExponentialDecayIter(int exponentialDecayIter);
    void setPerplexity(int perplexity);
    void setNumDimensionsOutput(int numDimensionsOutput);

    inline bool verbose() { return _verbose; }
    inline int iterations() { return _iterations; }
    inline int numTrees() { return _numTrees; }
    inline int numChecks() { return _numChecks; }
    inline int exaggerationIter() { return _exaggerationIter; }
    inline int perplexity() { return _perplexity; }
    inline int numDimensionsOutput() { return _numDimensionsOutput; }

    void initTSNE(std::vector<float>& data, const int numDimensions);
    void initWithProbDist(const int numPoints, const int numDimensions, const std::vector<hdi::data::MapMemEff<uint32_t, float>>& probDist);
    void stopGradientDescent();
    void markForDeletion();

    const TsneData& output();

    inline bool isTsneRunning() { return _isTsneRunning; }
    inline bool isGradientDescentRunning() { return _isGradientDescentRunning; }
    inline bool isMarkedForDeletion() { return _isMarkedForDeletion; }

    void reset();
    void startComputation();
    void stopComputation();

protected:
    // Hide QThread functionality, can't be inherited as protected because we need the public cast
    // TsneAnalysis to QObject. Exposing the cast instead does not seem to work.
    using QThread::start;

protected:
    void run() override;

private:
    void initGradientDescent(bool presetEmbedding);
    void embed();
    void copyFloatOutput();

signals:
    void newEmbedding();
    void computationStopped();
    void progressMessage(const QString& message);

private:
    // TSNE structures
    hdi::utils::knn_library _knnLibrary = hdi::utils::KNN_FLANN;
    hdi::utils::knn_distance_metric _knnDistanceMetric = hdi::utils::KNN_METRIC_EUCLIDEAN;
    hdi::dr::HDJointProbabilityGenerator<float>::sparse_scalar_matrix_type _probabilityDistribution;
    hdi::dr::SparseTSNEUserDefProbabilities<float> _A_tSNE;
    hdi::dr::GradientDescentTSNETexture _GPGPU_tSNE;
    hdi::data::Embedding<float> _embedding;

    // Data
    //TsneData _inputData;
    TsneData _outputData;
    unsigned int _numPoints;
    unsigned int _numDimensions;

    // Options
    int _iterations;
    int _numTrees;
    int _numChecks;
    int _exaggerationIter;
    int _exponentialDecayIter;
    int _perplexity;
    int _numDimensionsOutput;

    // Flags
    bool _verbose;
    bool _isGradientDescentRunning;
    bool _isTsneRunning;
    bool _isMarkedForDeletion;
    bool _stopThread;

    bool _gradientDescentInitialized;
    int _currentIteration;
};

class HDSimilarityComputationThread : public QThread
{
    Q_OBJECT
public:
    HDSimilarityComputationThread(std::vector<float>& data, int numPoints, int numDimensions, hdi::dr::HDJointProbabilityGenerator<float>::sparse_scalar_matrix_type& probDist, hdi::dr::HDJointProbabilityGenerator<float>::Parameters& params);

    void run() override;

signals:
    void resultReady(const QString& s);

private:
    std::vector<float>& _data;
    int _numPoints;
    int _numDimensions;
    hdi::dr::HDJointProbabilityGenerator<float>::sparse_scalar_matrix_type& _probDist;
    hdi::dr::HDJointProbabilityGenerator<float>::Parameters& _params;
};
