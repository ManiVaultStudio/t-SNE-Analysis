#include "HsneAnalysisPlugin.h"

#include "PointData.h"

#include <QtCore>
#include <QtDebug>

Q_PLUGIN_METADATA(IID "nl.tudelft.HsneAnalysisPlugin")
#ifdef _WIN32
#include <windows.h>
#endif

#include <set>

// =============================================================================
// View
// =============================================================================

using namespace hdps;

HsneAnalysisPlugin::HsneAnalysisPlugin() :
    AnalysisPlugin("H-SNE Analysis")
{

}

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

    registerDataEventByType(PointType, std::bind(&HsneAnalysisPlugin::onDataEvent, this, std::placeholders::_1));

    //connect(_settings.get(), &HsneSettingsWidget::stopComputation, this, &HsneAnalysisPlugin::stopComputation);
    //connect(&_tsne, &TsneAnalysis::computationStopped, _settings.get(), &HsneSettingsWidget::onComputationStopped);
    //connect(&_hsne._tsne, &TsneAnalysis::newEmbedding, this, &TsneAnalysisPlugin::onNewEmbedding);
    //connect(&_tsne, SIGNAL(newEmbedding()), this, SLOT(onNewEmbedding()));
}

void HsneAnalysisPlugin::onDataEvent(hdps::DataEvent* dataEvent)
{
    switch (dataEvent->getType())
    {
    case EventType::DataAdded:
    {
        _settings->addDataItem(dataEvent->dataSetName);
        break;
    }
    case EventType::DataChanged:
    {
        // If we are not looking at the changed dataset, ignore it
        if (dataEvent->dataSetName != _settings->getCurrentDataItem()) {
            break;
        }

        // Passes changes to the current dataset to the dimension selection widget
        Points& points = _core->requestData<Points>(dataEvent->dataSetName);

        _settings->getDimensionSelectionWidget().dataChanged(points);
        break;
    }
    case EventType::DataRemoved:
    {
        _settings->removeDataItem(dataEvent->dataSetName);
        break;
    }
    }
}

// If a different input dataset is picked in the settings widget update the dimension widget
void HsneAnalysisPlugin::dataSetPicked(const QString& name)
{
    Points& points = _core->requestData<Points>(name);

    _settings->getDimensionSelectionWidget().dataChanged(points);
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

    // Get the HSNE parameters from the settings widget
    HsneParameters parameters = _settings->getHsneParameters();

    std::vector<bool> enabledDimensions = _settings->getDimensionSelectionWidget().getEnabledDimensions();

    // Initialize the HSNE algorithm with the given parameters
    _hsne.initialize(_core, inputData, enabledDimensions, parameters);
    _hsne.setEmbeddingName(_settings->getEmbeddingName());
    _hsne.computeEmbedding(0);
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
