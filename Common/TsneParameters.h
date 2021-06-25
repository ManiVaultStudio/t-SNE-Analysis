#pragma once

class TsneParameters
{
public:
    TsneParameters() :
        _numIterations(1000),
        _perplexity(30),
        _numTrees(4),
        _numChecks(1024),
        _exaggerationIter(250),
        _exponentialDecayIter(150),
        _numDimensionsOutput(2)
    {

    }

    void setNumIterations(int numIterations) { _numIterations = numIterations; }
    void setPerplexity(int perplexity) { _perplexity = perplexity; }
    void setNumTrees(int numTrees) { _numTrees = numTrees; }
    void setNumChecks(int numChecks) { _numChecks = numChecks; }
    void setExaggerationIter(int exaggerationIter) { _exaggerationIter = exaggerationIter; }
    void setExponentialDecayIter(int exponentialDecayIter) { _exponentialDecayIter = exponentialDecayIter; }
    void setNumDimensionsOutput(int numDimensionsOutput) { _numDimensionsOutput = numDimensionsOutput; }

    int getNumIterations() { return _numIterations; }
    int getPerplexity() { return _perplexity; }
    int getNumTrees() { return _numTrees; }
    int getNumChecks() { return _numChecks; }
    int getExaggerationIter() { return _exaggerationIter; }
    int getExponentialDecayIter() { return _exponentialDecayIter; }
    int getNumDimensionsOutput() { return _numDimensionsOutput; }

private:
    int _numIterations;
    int _perplexity;
    int _numTrees;
    int _numChecks;
    int _exaggerationIter;
    int _exponentialDecayIter;
    int _numDimensionsOutput;
};
