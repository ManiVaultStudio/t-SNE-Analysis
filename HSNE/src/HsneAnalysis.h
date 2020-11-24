#pragma once

#include "CoreInterface.h"
#include "TsneAnalysis.h"
#include "hdi/dimensionality_reduction/hierarchical_sne.h"

#include <QString>
#include <QDebug>

#include <vector>
#include <memory>

using HsneMatrix = std::vector<hdi::data::MapMemEff<uint32_t, float>>;
using Hsne = hdi::dr::HierarchicalSNE<float, HsneMatrix>;

class Points;

class HsneParameters
{
public:
    HsneParameters() :
        _seed(-1),
        _useMonteCarloSampling(true),
        _numWalksForLandmarkSelection(15),
        _numWalksForLandmarkSelectionThreshold(1.5f),
        _randomWalkLength(15),
        _numWalksForAreaOfInfluence(100),
        _minWalksRequired(0),
        _numChecksAknn(512),
        _useOutOfCoreComputation(true)
    {

    }

    void setSeed(int seed) { _seed = seed; }
    void setNumWalksForLandmarkSelection(int numWalks) { _numWalksForLandmarkSelection = numWalks; }
    void setNumWalksForLandmarkSelectionThreshold(float numWalks) { _numWalksForLandmarkSelectionThreshold = numWalks; }
    void setRandomWalkLength(int length) { _randomWalkLength = length; }
    void setNumWalksForAreaOfInfluence(int numWalks) { _numWalksForAreaOfInfluence = numWalks; }
    void setMinWalksRequired(int minWalks) { _minWalksRequired = minWalks; }
    void setNumChecksAKNN(int numChecks) { _numChecksAknn = numChecks; }
    void useMonteCarloSampling(bool useMonteCarloSampling) { _useMonteCarloSampling = useMonteCarloSampling; }
    void useOutOfCoreComputation(bool useOutOfCoreComputation) { _useOutOfCoreComputation = useOutOfCoreComputation; }

    int getSeed() { return _seed; }
    int getNumWalksForLandmarkSelection() { return _numWalksForLandmarkSelection; }
    float getNumWalksForLandmarkSelectionThreshold() { return _numWalksForLandmarkSelectionThreshold; }
    int getRandomWalkLength() { return _randomWalkLength; }
    int getNumWalksForAreaOfInfluence() { return _numWalksForAreaOfInfluence; }
    int getMinWalksRequired() { return _minWalksRequired; }
    int getNumChecksAKNN() { return _numChecksAknn; }
    bool useMonteCarloSampling() { return _useOutOfCoreComputation; }
    bool useOutOfCoreComputation() { return _useOutOfCoreComputation; }
private:
    /** Seed used for random algorithms. If a negative value is provided a time based seed is used */
    int _seed;
    bool _useMonteCarloSampling;
    int _numWalksForLandmarkSelection;
    float _numWalksForLandmarkSelectionThreshold;
    int _randomWalkLength;
    int _numWalksForAreaOfInfluence;
    int _minWalksRequired;
    int _numChecksAknn;
    bool _useOutOfCoreComputation;
};

class HsneAnalysis : public QObject
{
    Q_OBJECT
public:
    /**
     * Initialize the HSNE hierarchy with a data-level scale.
     *
     * @param  data        The high-dimensional data
     * @param  parameters  Parameters with which to run the HSNE algorithm
     */
    void initialize(hdps::CoreInterface* core, const Points& inputData, const std::vector<bool>& enabledDimensions, const HsneParameters& parameters);
    void computeEmbedding(int scale = 0);
    //TsneAnalysis& getTsneAnalysis() { return _tsne; }

    TsneAnalysis _tsne;
    void setEmbeddingName(QString embeddingName);

private:
    QString createEmptyEmbedding(QString name, QString dataType, QString sourceName);

public slots:
    void newScale();
    void onNewEmbedding();

private:
    hdps::CoreInterface* _core;

    std::unique_ptr<Hsne> _hsne;

    QString _inputDataName;
    QString _embeddingNameBase;
    QString _embeddingName;

    // TEMP
    unsigned int _numPoints;
    unsigned int _numDimensions;
    int scale = 0;
};
