#include "HsneAnalysisPlugin.h"

#include "PointData.h"

#include <QtCore>
#include <QtDebug>

Q_PLUGIN_METADATA(IID "nl.tudelft.HsneAnalysisPlugin")
#include <windows.h>

#include <set>

// =============================================================================
// View
// =============================================================================

using namespace hdps;

HsneAnalysisPlugin::~HsneAnalysisPlugin(void)
{
    
}

void HsneAnalysisPlugin::init()
{
    // Create a new settings widget which allows users to change the parameters given to the HSNE analysis
    _settings = std::make_unique<HsneSettingsWidget>();

    // If a different input dataset is picked in the settings widget update the dimension widget
    connect(_settings.get(), &HsneSettingsWidget::dataSetPicked, this, &HsneAnalysisPlugin::dataSetPicked);
    // If the start computation button is pressed, run the HSNE algorithm
    connect(_settings.get(), &HsneSettingsWidget::startComputation, this, &HsneAnalysisPlugin::startComputation);

    //connect(_settings.get(), &HsneSettingsWidget::stopComputation, this, &HsneAnalysisPlugin::stopComputation);
    //connect(&_tsne, &TsneAnalysis::computationStopped, _settings.get(), &HsneSettingsWidget::onComputationStopped);
    //connect(&_hsne._tsne, &TsneAnalysis::newEmbedding, this, &TsneAnalysisPlugin::onNewEmbedding);
    //connect(&_tsne, SIGNAL(newEmbedding()), this, SLOT(onNewEmbedding()));
}

void HsneAnalysisPlugin::dataAdded(const QString name)
{
    _settings->addDataItem(name);
}

void HsneAnalysisPlugin::dataChanged(const QString name)
{
    // If we are not looking at the changed dataset, ignore it
    if (name != _settings->getCurrentDataItem()) {
        return;
    }

    // Passes changes to the current dataset to the dimension selection widget
    Points& points = _core->requestData<Points>(name);

    _settings->getDimensionSelectionWidget().dataChanged(points);
}

void HsneAnalysisPlugin::dataRemoved(const QString name)
{
    _settings->removeDataItem(name);
}

void HsneAnalysisPlugin::selectionChanged(const QString dataName)
{
    // Unused in analysis
}

DataTypes HsneAnalysisPlugin::supportedDataTypes() const
{
    DataTypes supportedTypes;
    supportedTypes.append(PointType);
    return supportedTypes;
}

SettingsWidget* const HsneAnalysisPlugin::getSettings()
{
    return _settings.get();
}

// If a different input dataset is picked in the settings widget update the dimension widget
void HsneAnalysisPlugin::dataSetPicked(const QString& name)
{
    Points& points = _core->requestData<Points>(name);

    _settings->getDimensionSelectionWidget().dataChanged(points);
}

Points& createEmptyEmbedding(CoreInterface* core, QString name, QString dataType, const Points& source)
{
    QString embeddingName = core->createDerivedData(dataType, name, source.getName());
    Points& embedding = core->requestData<Points>(embeddingName);
    embedding.setData(nullptr, 0, 2);
    core->notifyDataAdded(embeddingName);
    return embedding;
}

void HsneAnalysisPlugin::startComputation()
{
    //initializeTsne();

    /********************/
    /* Prepare the data */
    /********************/
    // Obtain a reference to the the input dataset
    QString setName = _settings->getCurrentDataItem();
    const Points& inputData = _core->requestData<Points>(setName);

    // Extract the enabled dimensions from the data
    std::vector<bool> enabledDimensions = _settings->getDimensionSelectionWidget().getEnabledDimensions();
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

    // Create new data set for the embedding
    createEmptyEmbedding(_core, "Embedding", "Points", inputData);

    // Get the HSNE parameters from the settings widget
    HsneParameters parameters = _settings->getHsneParameters();

    // Initialize the HSNE algorithm with the given parameters
    _hsne.initialize(data, inputData.getNumPoints(), numDimensions, parameters);

    _hsne.computeEmbedding();
}

//void HsneAnalysisPlugin::onNewEmbedding() {
//
//    const TsneData& outputData = _tsne.output();
//    Points& embedding = _core->requestData<Points>(_embeddingName);
//
//    embedding.setData(outputData.getData().data(), outputData.getNumPoints(), 2);
//
//    _core->notifyDataChanged(_embeddingName);
//}

// =============================================================================
// Factory
// =============================================================================

AnalysisPlugin* HsneAnalysisPluginFactory::produce()
{
    return new HsneAnalysisPlugin();
}
