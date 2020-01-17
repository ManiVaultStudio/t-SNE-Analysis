#pragma once

#include "hdi/dimensionality_reduction/hierarchical_sne.h"

#include <vector>
#include <memory>

using HsneMatrix = std::vector<hdi::data::MapMemEff<uint32_t, float>>;
using Hsne = hdi::dr::HierarchicalSNE<float, HsneMatrix>;

class HsneParameters
{
public:
    int getSeed() { return _seed; }
    int getNumWalksForLandmarkSelection() { return _numWalksForLandmarkSelection; }
    int getNumWalksForLandmarkSelectionThreshold() { return _numWalksForLandmarkSelectionThreshold; }
    int getRandomWalkLength() { return _randomWalkLength; }
    int getNumWalksForAreaOfInfluence() { return _numWalksForAreaOfInfluence; }
    int getMinWalksRequired() { return _minWalksRequired; }
    int getNumChecksAKNN() { return _numChecksAknn; }
    bool usesMonteCarloSampling() { return _useOutOfCoreComputation; }
    bool doesOutOfCoreComputation() { return _useOutOfCoreComputation; }
private:
    /** Seed used for random algorithms. If a negative value is provided a time based seed is used */
    int _seed;
    bool _useMonteCarloSampling;
    int _numWalksForLandmarkSelection;
    int _numWalksForLandmarkSelectionThreshold;
    int _randomWalkLength;
    int _numWalksForAreaOfInfluence;
    int _minWalksRequired;
    int _numChecksAknn;
    bool _useOutOfCoreComputation;
};

class HsneAnalysis
{
public:
    /**
     * Initialize the HSNE hierarchy with a data-level scale.
     *
     * @param  data        The high-dimensional data
     * @param  parameters  Parameters with which to run the HSNE algorithm
     */
    void initialize(const std::vector<float>& data, const HsneParameters& parameters);
    void computeEmbedding();

private:
    std::unique_ptr<Hsne> _hsne;
};
