#pragma once

#include "TsneParameters.h"
#include "TsneData.h"

#include "hdi/dimensionality_reduction/hd_joint_probability_generator.h"
#include "hdi/dimensionality_reduction/sparse_tsne_user_def_probabilities.h"
#include "hdi/dimensionality_reduction/gradient_descent_tsne_texture.h"
#include "hdi/dimensionality_reduction/knn_utils.h"

#include <QThread>

#include <vector>
#include <string>

class TsneAnalysis : public QThread
{
    Q_OBJECT
public:
    TsneAnalysis();
    ~TsneAnalysis() override;

    void setParameters(TsneParameters parameters);

    void setVerbose(bool verbose);

    inline bool verbose() { return _verbose; }

    void initTSNE(std::vector<float>& data, const int numDimensions);
    void initWithProbDist(const int numPoints, const int numDimensions, const std::vector<hdi::data::MapMemEff<uint32_t, float>>& probDist);
    void stopGradientDescent();
    void markForDeletion();

    const TsneData& output();

    inline bool isTsneRunning() { return _isTsneRunning; }
    inline bool isGradientDescentRunning() { return _isGradientDescentRunning; }
    inline bool isMarkedForDeletion() { return _isMarkedForDeletion; }

private:
    void run() override;

    void computeGradientDescent();
    void initGradientDescent();
    void embed();
    void copyFloatOutput();

signals:
    void newEmbedding();
    void computationStopped();
    void progressMessage(const QString& message);

private:
    // TSNE structures
    hdi::dr::HDJointProbabilityGenerator<float>::sparse_scalar_matrix_type _probabilityDistribution;
    hdi::dr::GradientDescentTSNETexture _GPGPU_tSNE;
    hdi::data::Embedding<float> _embedding;

    // Data
    //TsneData _inputData;
    TsneData _outputData;
    unsigned int _numPoints;
    unsigned int _numDimensions;

    // Options
    TsneParameters _tsneParameters;

    // Flags
    bool _verbose;
    bool _isGradientDescentRunning;
    bool _isTsneRunning;
    bool _isMarkedForDeletion;

    int _continueFromIteration;
};
