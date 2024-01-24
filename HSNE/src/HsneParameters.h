#pragma once

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
        _numScales(3),
        _seed(-1),
        _useMonteCarloSampling(true),
        _numWalksForLandmarkSelection(15),
        _numWalksForLandmarkSelectionThreshold(1.5f),
        _randomWalkLength(15),
        _numWalksForAreaOfInfluence(100),
        _minWalksRequired(0),
        _useOutOfCoreComputation(true),
        _saveHierarchyToDisk(true),
        _numNeighbors(90)
    {

    }

    // Basic options
    void setNumScales(unsigned int numScales) { _numScales = numScales; }
    void setSeed(int seed) { _seed = seed; }

    unsigned int getNumScales() const { return _numScales; }
    int getSeed() const { return _seed; }

    // Advanced options
    void setNumWalksForLandmarkSelection(int numWalks) { _numWalksForLandmarkSelection = numWalks; }
    void setNumWalksForLandmarkSelectionThreshold(float numWalks) { _numWalksForLandmarkSelectionThreshold = numWalks; }
    void setRandomWalkLength(int length) { _randomWalkLength = length; }
    void setNumWalksForAreaOfInfluence(int numWalks) { _numWalksForAreaOfInfluence = numWalks; }
    void setMinWalksRequired(int minWalks) { _minWalksRequired = minWalks; }
    void setNumNearestNeighbors(int numNeighbors) { _numNeighbors = numNeighbors; }
    void useMonteCarloSampling(bool useMonteCarloSampling) { _useMonteCarloSampling = useMonteCarloSampling; }
    void useOutOfCoreComputation(bool useOutOfCoreComputation) { _useOutOfCoreComputation = useOutOfCoreComputation; }

    int getNumWalksForLandmarkSelection() const { return _numWalksForLandmarkSelection; }
    float getNumWalksForLandmarkSelectionThreshold() const { return _numWalksForLandmarkSelectionThreshold; }
    int getRandomWalkLength() const { return _randomWalkLength; }
    int getNumNearestNeighbors() const { return _numNeighbors; }
    int getNumWalksForAreaOfInfluence() const { return _numWalksForAreaOfInfluence; }
    int getMinWalksRequired() const { return _minWalksRequired; }
    bool useMonteCarloSampling() const { return _useOutOfCoreComputation; }
    bool useOutOfCoreComputation() const { return _useOutOfCoreComputation; }

    // Plugin specific

    void setSaveHierarchyToDisk(bool saveHierarchyToDisk) { _saveHierarchyToDisk = saveHierarchyToDisk; }
    bool getSaveHierarchyToDisk() const { return _saveHierarchyToDisk; }

private:
    // Basic
    
    unsigned int _numScales;                        /** Number of scales the hierarchy should consist of */
    int _seed;                                      /** Seed used for random algorithms. If a negative value is provided a time based seed is used */

    // Advanced
    
    bool _useMonteCarloSampling;                    /** Whether to use markov chain monte carlo sampling for computing the landmarks */
    int _numWalksForLandmarkSelection;              /** How many random walks to use in the MCMC sampling process */
    float _numWalksForLandmarkSelectionThreshold;   /** How many times a landmark should be the endpoint of a random walk before it is considered a landmark */
    int _randomWalkLength;                          /** How long each random walk should be */
    int _numWalksForAreaOfInfluence;                /** How many random walks to use for computing the area of influence */
    int _minWalksRequired;                          /** Minimum number of walks to be considered in the computation of the transition matrix */
    bool _useOutOfCoreComputation;                  /** Preserve memory while computing the hierarchy */
    int _numNeighbors;                              /** Number nearest neighbors. In HDI internally it'll use nn = _numNeighbors + 1 and perplexity = _numNeighbors / 3 */

    // Plugin specific

    bool _saveHierarchyToDisk;                      /** Save hierarchy to disk */
};
