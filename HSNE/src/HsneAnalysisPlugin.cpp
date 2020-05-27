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
    stopComputation();
}

void HsneAnalysisPlugin::init()
{
    _settings = std::make_unique<HsneSettingsWidget>();

    connect(_settings.get(), &HsneSettingsWidget::dataSetPicked, this, &HsneAnalysisPlugin::dataSetPicked);
    connect(_settings.get(), &HsneSettingsWidget::startComputation, this, &HsneAnalysisPlugin::startComputation);
    connect(_settings.get(), &HsneSettingsWidget::stopComputation, this, &HsneAnalysisPlugin::stopComputation);
    connect(&_tsne, &TsneAnalysis::computationStopped, _settings.get(), &HsneSettingsWidget::onComputationStopped);
    //connect(&_hsne._tsne, &TsneAnalysis::newEmbedding, this, &TsneAnalysisPlugin::onNewEmbedding);
    connect(&_tsne, SIGNAL(newEmbedding()), this, SLOT(onNewEmbedding()));
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

void HsneAnalysisPlugin::dataSetPicked(const QString& name)
{
    Points& points = _core->requestData<Points>(name);

    _settings->getDimensionSelectionWidget().dataChanged(points);
}

void HsneAnalysisPlugin::startComputation()
{
    initializeTsne();

    // Prepare the data
    QString setName = _settings->getCurrentDataItem();
    const Points& points = _core->requestData<Points>(setName);

    // Create list of data from the enabled dimensions
    std::vector<float> data;

    auto selection = points.indices;

    // If the dataset is not a subset, use all data points
    if (points.isFull()) {
        std::vector<std::uint32_t> all(points.getNumPoints());
        std::iota(std::begin(all), std::end(all), 0);

        selection = all;
    }

    // Extract the enabled dimensions from the data
    std::vector<bool> enabledDimensions = _settings->getDimensionSelectionWidget().getEnabledDimensions();
    unsigned int numDimensions = count_if(enabledDimensions.begin(), enabledDimensions.end(), [](bool b) { return b; });

    data.reserve(selection.size() * numDimensions);

    for (const auto& pointId : selection)
    {
        for (int dimensionId = 0; dimensionId < points.getNumDimensions(); dimensionId++)
        {
            if (enabledDimensions[dimensionId]) {
                const auto index = pointId * points.getNumDimensions() + dimensionId;
                data.push_back(points[index]);
            }
        }
    }

    // Create new data set for the embedding
    _embeddingName = _core->createDerivedData("Points", "Embedding", points.getName());
    Points& embedding = _core->requestData<Points>(_embeddingName);
    embedding.setData(nullptr, 0, 2);
    _core->notifyDataAdded(_embeddingName);

    _tsne.initTSNE(data, numDimensions);

    _tsne.start();
}

void HsneAnalysisPlugin::onNewEmbedding() {

    const TsneData& outputData = _tsne.output();
    Points& embedding = _core->requestData<Points>(_embeddingName);

    embedding.setData(outputData.getData().data(), outputData.getNumPoints(), 2);

    _core->notifyDataChanged(_embeddingName);
}

void HsneAnalysisPlugin::initializeTsne() {
    // Initialize the tSNE computation with the settings from the settings widget
    //_tsne.setIterations(_settings->numIterations.text().toInt());
    //_tsne.setPerplexity(_settings->perplexity.text().toInt());
    //_tsne.setExaggerationIter(_settings->exaggeration.text().toInt());
    //_tsne.setNumTrees(_settings->numTrees.text().toInt());
    //_tsne.setNumChecks(_settings->numChecks.text().toInt());
    _tsne.setIterations(1000);
    _tsne.setPerplexity(30);
    _tsne.setExaggerationIter(250);
    _tsne.setNumTrees(4);
    _tsne.setNumChecks(1024);
}

void HsneAnalysisPlugin::stopComputation() {
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
}

// =============================================================================
// Factory
// =============================================================================

AnalysisPlugin* HsneAnalysisPluginFactory::produce()
{
    return new HsneAnalysisPlugin();
}
