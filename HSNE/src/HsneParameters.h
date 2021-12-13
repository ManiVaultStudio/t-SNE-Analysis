#pragma once

#include "hdi/dimensionality_reduction/knn_utils.h"

/**
 * HsneParameters
 *
 * Container class for the parameters associated with the HSNE algorithm
 *
 * @author Julian Thijssen
 */
class HsneParameters
{
public:
    HsneParameters() :
        _knnLibrary(hdi::dr::KNN_FLANN),
        _numScales(3),
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

    // Basic options
    void setKnnLibrary(hdi::dr::knn_library library) { _knnLibrary = library; }
    void setNumScales(unsigned int numScales) { _numScales = numScales; }
    void setSeed(int seed) { _seed = seed; }

    hdi::dr::knn_library getKnnLibrary() const { return _knnLibrary; }
    unsigned int getNumScales() const { return _numScales; }
    int getSeed() const { return _seed; }

    // Advanced options
    void setNumWalksForLandmarkSelection(int numWalks) { _numWalksForLandmarkSelection = numWalks; }
    void setNumWalksForLandmarkSelectionThreshold(float numWalks) { _numWalksForLandmarkSelectionThreshold = numWalks; }
    void setRandomWalkLength(int length) { _randomWalkLength = length; }
    void setNumWalksForAreaOfInfluence(int numWalks) { _numWalksForAreaOfInfluence = numWalks; }
    void setMinWalksRequired(int minWalks) { _minWalksRequired = minWalks; }
    void setNumChecksAKNN(int numChecks) { _numChecksAknn = numChecks; }
    void useMonteCarloSampling(bool useMonteCarloSampling) { _useMonteCarloSampling = useMonteCarloSampling; }
    void useOutOfCoreComputation(bool useOutOfCoreComputation) { _useOutOfCoreComputation = useOutOfCoreComputation; }

    int getNumWalksForLandmarkSelection() const { return _numWalksForLandmarkSelection; }
    float getNumWalksForLandmarkSelectionThreshold() const { return _numWalksForLandmarkSelectionThreshold; }
    int getRandomWalkLength() const { return _randomWalkLength; }
    int getNumWalksForAreaOfInfluence() const { return _numWalksForAreaOfInfluence; }
    int getMinWalksRequired() const { return _minWalksRequired; }
    int getNumChecksAKNN() const { return _numChecksAknn; }
    bool useMonteCarloSampling() const { return _useOutOfCoreComputation; }
    bool useOutOfCoreComputation() const { return _useOutOfCoreComputation; }

private:
    // Basic
    /** Enum specifying which approximate nearest neighbour library to use for the similarity computation */
    hdi::dr::knn_library _knnLibrary;
    /** Number of scales the hierarchy should consist of */
    unsigned int _numScales;
    /** Seed used for random algorithms. If a negative value is provided a time based seed is used */
    int _seed;

    // Advanced
    /** Whether to use markov chain monte carlo sampling for computing the landmarks */
    bool _useMonteCarloSampling;
    /** How many random walks to use in the MCMC sampling process */
    int _numWalksForLandmarkSelection;
    /** How many times a landmark should be the endpoint of a random walk before it is considered a landmark */
    float _numWalksForLandmarkSelectionThreshold;
    /** How long each random walk should be */
    int _randomWalkLength;
    /** How many random walks to use for computing the area of influence */
    int _numWalksForAreaOfInfluence;
    /** Minimum number of walks to be considered in the computation of the transition matrix */
    int _minWalksRequired;
    /** Number of checks used in the ANN computation, more checks means more precision but slower computation */
    int _numChecksAknn;
    /** Preserve memory while computing the hierarchy */
    bool _useOutOfCoreComputation;
};
