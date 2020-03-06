#include "HsneAnalysis.h"

//using HsneMatrix = std::vector<hdi::data::MapMemEff<uint32_t, float>>;
//using HsneInternalParams = hdi::dr::HierarchicalSNE<float, HsneMatrix>::Parameters;

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

void HsneAnalysis::initialize(const std::vector<float>& data, unsigned int numPoints, unsigned int numDimensions, const HsneParameters& parameters)
{
    // TEMP
    connect(&_tsne, &TsneAnalysis::computationStopped, this, &HsneAnalysis::newScale);

    // Set parameters
    Hsne::Parameters internalParams = setParameters(parameters);

    // Prepare data
    int numScales = 5; // FIXME Should be some param
    _numDimensions = numDimensions;

    // Initialize hierarchy
    _hsne = std::make_unique<Hsne>();
    //_hsne->initialize(data, parameters);
    hdi::utils::CoutLog _log;
    _hsne->setLogger(&_log);
    _hsne->setDimensionality(numDimensions);
    //for (int i = 0; i < 10; i++)
    //{
    //    std::cout << data[i] << std::endl;
    //}
    _hsne->initialize((Hsne::scalar_type*) data.data(), numPoints, internalParams);

    for (int s = 0; s < numScales - 1; ++s) {
        _hsne->addScale();
    }

    // Initialize tsne
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

void HsneAnalysis::newScale() { scale++; std::cout << "New scale!" << std::endl;  computeEmbedding(); }

void HsneAnalysis::computeEmbedding()
{
    if (_tsne.isRunning())
    {
        // Request interruption of the computation
        _tsne.stopGradientDescent();
        _tsne.exit();

        // Wait until the thread has terminated (max. 3 seconds)
        if (!_tsne.wait(3000))
        {
            qDebug() << "tSNE computation thread did not close in time, terminating...";
            _tsne.terminate();
            _tsne.wait();
        }
        qDebug() << "tSNE computation stopped.";
    }
    // Initialize tsne, compute data to be embedded, start computation?
    _tsne.initWithProbDist(1, _numDimensions, _hsne->scale(scale)._transition_matrix); // FIXME
    // Embed data
    _tsne.start();
}
