#pragma once

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
