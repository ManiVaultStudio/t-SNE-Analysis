#include "HsneAnalysis.h"

//using HsneMatrix = std::vector<hdi::data::MapMemEff<uint32_t, float>>;
//using HsneInternalParams = hdi::dr::HierarchicalSNE<float, HsneMatrix>::Parameters;

namespace
{
    Hsne::Parameters setParameters(HsneParameters parameters)
    {
        Hsne::Parameters params;
        params._seed = parameters.getSeed();
        params._num_walks_per_landmark = parameters.getNumWalksForAreaOfInfluence();
        params._monte_carlo_sampling = parameters.usesMonteCarloSampling();
        params._mcmcs_num_walks = parameters.getNumWalksForLandmarkSelection();
        params._mcmcs_landmark_thresh = parameters.getNumWalksForLandmarkSelectionThreshold();
        params._mcmcs_walk_length = parameters.getRandomWalkLength();
        params._transition_matrix_prune_thresh = parameters.getMinWalksRequired();
        params._aknn_num_checks = parameters.getNumChecksAKNN();
        params._out_of_core_computation = parameters.doesOutOfCoreComputation();
        return params;
    }
}

void HsneAnalysis::initialize(const std::vector<float>& data, const HsneParameters& parameters)
{
    // Set parameters
    Hsne::Parameters internalParams = setParameters(parameters);

    // Create hierarchy
    //_hierarchy = new MCV_DerivedDataHierarchy();
    //_hierarchy->init(_name, _numDataPoints, numDimensions, _numScales, params, _dataSelection->data());

    // Prepare data
    int numPoints = 1; // FIXME should be some param
    int numDimensions = 1; // FIXME should be some param
    int numScales = 1; // FIXME Should be some param

    // Initialize hierarchy
    _hsne = std::make_unique<Hsne>();
    //_hsne->initialize(data, parameters);
    _hsne->setDimensionality(numDimensions);
    _hsne->initialize((Hsne::scalar_type*) data.data(), numPoints, internalParams);

    for (int s = 0; s < numScales - 1; ++s) {
        _hsne->addScale();
    }

    // New hd prob matrix, new embedding, empty transition matrix


    // Set scale, clear embedding, set transition matrix
}

void HsneAnalysis::computeEmbedding()
{
    // Initialize tsne, compute data to be embedded, start computation?

    // Embed data
}
