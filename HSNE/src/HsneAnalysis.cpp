#include "HsneAnalysis.h"

#include "PointData.h"

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

void HsneAnalysis::initialize(hdps::CoreInterface* core, const Points& inputData, const std::vector<bool>& enabledDimensions, const HsneParameters& parameters)
{
    _core = core;

    // TEMP temporary connection to call the newScale slot when embedding is finished
    connect(&_tsne, &TsneAnalysis::computationStopped, this, &HsneAnalysis::newScale);
    connect(&_tsne, &TsneAnalysis::newEmbedding, this, &HsneAnalysis::onNewEmbedding);

    // Convert our own HSNE parameters to the HDI parameters
    Hsne::Parameters internalParams = setParameters(parameters);

    // Prepare data
    _inputDataName = inputData.getName();

    // Extract the enabled dimensions from the data
    unsigned int numDimensions = count_if(enabledDimensions.begin(), enabledDimensions.end(), [](bool b) { return b; });

    std::vector<float> data;
    {
        auto selection = inputData.indices;

        // If the dataset is not a subset, use all data points
        if (inputData.isFull()) {
            std::vector<std::uint32_t> all(inputData.getNumPoints());
            std::iota(std::begin(all), std::end(all), 0);

            selection = all;
        }

        data.reserve(selection.size() * numDimensions);

        inputData.visitFromBeginToEnd([&data, &selection, &enabledDimensions, numDimensions](auto beginOfData, auto endOfData)
        {
            for (const auto& pointId : selection)
            {
                for (int dimensionId = 0; dimensionId < numDimensions; dimensionId++)
                {
                    if (enabledDimensions[dimensionId]) {
                        const auto index = pointId * numDimensions + dimensionId;
                        data.push_back(beginOfData[index]);
                    }
                }
            }
        });
    }
    //inputData.populateDataForDimensions(data, enabledDimensions);

    int numScales = 5; // FIXME Should be some param
    _numPoints = inputData.getNumPoints();
    _numDimensions = numDimensions;

    // Initialize hierarchy
    _hsne = std::make_unique<Hsne>();
    //_hsne->initialize(data, parameters);

    // Set up a logger
    hdi::utils::CoutLog _log;
    _hsne->setLogger(&_log);

    // Set the dimensionality of the data in the HSNE object
    _hsne->setDimensionality(numDimensions);

    // Initialize HSNE with the input data and the given parameters
    _hsne->initialize((Hsne::scalar_type*) data.data(), _numPoints, internalParams);

    // Add a number of scales as indicated by the user
    for (int s = 0; s < numScales - 1; ++s) {
        _hsne->addScale();
    }

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

void HsneAnalysis::setEmbName(QString embName)
{
	_embeddingNameBase = embName;
}


void HsneAnalysis::newScale() {
    scale++; std::cout << "New scale!" << std::endl;

    if (scale < 5)
    {
        computeEmbedding(scale);
    }
}

void HsneAnalysis::onNewEmbedding() {
    const TsneData& outputData = _tsne.output();
    Points& embedding = _core->requestData<Points>(_embeddingName);

    embedding.setData(outputData.getData().data(), outputData.getNumPoints(), 2);

    _core->notifyDataChanged(_embeddingName);
}

QString HsneAnalysis::createEmptyEmbedding(QString name, QString dataType, QString sourceName)
{
    QString embeddingName = _core->createDerivedData(dataType, name, sourceName);
    Points& embedding = _core->requestData<Points>(embeddingName);
    embedding.setData(nullptr, 0, 2);
    _core->notifyDataAdded(embeddingName);
    return embeddingName;
}

void HsneAnalysis::computeEmbedding(int lvlIndicator)
{
    // Create a new data set for the embedding
    _embeddingName = createEmptyEmbedding(_embeddingNameBase + "_scale_" + QString::number(lvlIndicator), "Points", _inputDataName);

    // Should come from some t-SNE settings widget
    _tsne.setIterations(1000);
    _tsne.setPerplexity(30);
    _tsne.setExaggerationIter(250);
    _tsne.setNumTrees(4);
    _tsne.setNumChecks(1024);

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
    _tsne.initWithProbDist(_numPoints, _numDimensions, _hsne->scale(scale)._transition_matrix); // FIXME
    // Embed data
    _tsne.start();
}
