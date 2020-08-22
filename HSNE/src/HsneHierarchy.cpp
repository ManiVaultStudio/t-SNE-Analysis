#include "HsneHierarchy.h"

#include "HsneParameters.h"
#include "PointData.h"

#include "hdi/utils/cout_log.h"
#include <iostream>

namespace
{
    Hsne::Parameters setParameters(HsneParameters parameters)
    {
        Hsne::Parameters params;
        params._seed = parameters.getSeed();
        params._num_walks_per_landmark = parameters.getNumWalksForAreaOfInfluence();
        params._monte_carlo_sampling = parameters.useMonteCarloSampling();
        params._mcmcs_num_walks = parameters.getNumWalksForLandmarkSelection();
        params._mcmcs_landmark_thresh = parameters.getNumWalksForLandmarkSelectionThreshold();
        params._mcmcs_walk_length = parameters.getRandomWalkLength();
        params._transition_matrix_prune_thresh = parameters.getMinWalksRequired();
        params._aknn_num_checks = parameters.getNumChecksAKNN();
        params._out_of_core_computation = parameters.useOutOfCoreComputation();
        return params;
    }
}

void HsneHierarchy::initialize(hdps::CoreInterface* core, const Points& inputData, const std::vector<bool>& enabledDimensions, const HsneParameters& parameters)
{
    _core = core;

    // TEMP temporary connection to call the newScale slot when embedding is finished
    //connect(&_tsne, &TsneAnalysis::computationStopped, this, &HsneHierarchy::newScale);
    //connect(&_tsne, &TsneAnalysis::newEmbedding, this, &HsneHierarchy::onNewEmbedding);

    // Convert our own HSNE parameters to the HDI parameters
    Hsne::Parameters internalParams = setParameters(parameters);

    // Prepare data
    _inputDataName = inputData.getName();

    // Create list of data from the enabled dimensions
    std::vector<float> data;
    std::vector<unsigned int> indices;

    // Extract the enabled dimensions from the data
    unsigned int numEnabledDimensions = count_if(enabledDimensions.begin(), enabledDimensions.end(), [](bool b) { return b; });
    data.resize((inputData.isFull() ? inputData.getNumPoints() : inputData.indices.size()) * numEnabledDimensions);
    for (int i = 0; i < inputData.getNumDimensions(); i++)
        if (enabledDimensions[i]) indices.push_back(i);

    inputData.populateDataForDimensions<std::vector<float>, std::vector<unsigned int>>(data, indices);

    int numScales = 3; // FIXME Should be some param
    _numPoints = inputData.getNumPoints();
    _numDimensions = numEnabledDimensions;

    // Initialize hierarchy
    _hsne = std::make_unique<Hsne>();
    //_hsne->initialize(data, parameters);

    // Set up a logger
    hdi::utils::CoutLog _log;
    _hsne->setLogger(&_log);

    // Set the dimensionality of the data in the HSNE object
    _hsne->setDimensionality(numEnabledDimensions);

    // Initialize HSNE with the input data and the given parameters
    _hsne->initialize((Hsne::scalar_type*) data.data(), _numPoints, internalParams);

    // Add a number of scales as indicated by the user
    for (int s = 0; s < numScales - 1; ++s) {
        _hsne->addScale();
    }

    _currentScale = 2;
    // Initialize t-SNE
    //_tsne.initWithProbDist(data, numDimensions, _hsne->scale(4)._transition_matrix);

    // New hd prob matrix, new embedding, empty transition matrix
    // Set scale, clear embedding, set transition matrix
    //for (int i = 0; i < numPoints; i++)
    //{
    //    for (int j = 0; j < numPoints; j++)
    //    {
    //        if (_hsne->scale(0)._transition_matrix[i][j] != 0)
    //            std::cout << _hsne->scale(0)._transition_matrix[i][j] << std::endl;
    //    }
    //}
}

void HsneHierarchy::newScale() {
    //scale++; std::cout << "New scale!" << std::endl;

    //if (scale < 3)
    //{
    //    computeEmbedding();
    //}
}

//void HsneHierarchy::onNewEmbedding() {
//    const TsneData& outputData = _tsne.output();
//    Points& embedding = _core->requestData<Points>(_embeddingName);
//
//    embedding.setData(outputData.getData().data(), outputData.getNumPoints(), 2);
//
//    _core->notifyDataChanged(_embeddingName);
//}
