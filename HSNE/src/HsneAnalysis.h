#pragma once

#include "TsneAnalysis.h"
#include "hdi/dimensionality_reduction/hierarchical_sne.h"

#include <vector>
#include <memory>
#include <QDebug>

using HsneMatrix = std::vector<hdi::data::MapMemEff<uint32_t, float>>;
using Hsne = hdi::dr::HierarchicalSNE<float, HsneMatrix>;

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
    void initialize(const std::vector<float>& data, unsigned int numPoints, unsigned int numDimensions, const HsneParameters& parameters);
    void computeEmbedding();

    TsneAnalysis _tsne;

public slots:
    void newScale();
private:
    std::unique_ptr<Hsne> _hsne;

    // TEMP
    unsigned int _numDimensions;
    int scale = 0;
};
