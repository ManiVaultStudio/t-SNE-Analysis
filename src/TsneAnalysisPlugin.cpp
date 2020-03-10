#include "TsneAnalysisPlugin.h"
#include "TsneSettingsWidget.h"

#include "PointData.h"

#include <QtCore>
#include <QtDebug>

Q_PLUGIN_METADATA(IID "nl.tudelft.TsneAnalysisPlugin")
#include <windows.h>

#include <set>

// =============================================================================
// View
// =============================================================================

using namespace hdps;
TsneAnalysisPlugin::TsneAnalysisPlugin()
:
AnalysisPlugin("tSNE Analysis")
{
}

TsneAnalysisPlugin::~TsneAnalysisPlugin(void)
{
    stopComputation();
}

void TsneAnalysisPlugin::init()
{
    _settings = std::make_unique<TsneSettingsWidget>(*this);

    connect(_settings.get(), &TsneSettingsWidget::dataSetPicked, this, &TsneAnalysisPlugin::dataSetPicked);
    connect(_settings.get(), &TsneSettingsWidget::knnAlgorithmPicked, this, &TsneAnalysisPlugin::onKnnAlgorithmPicked);
    connect(_settings.get(), &TsneSettingsWidget::distanceMetricPicked, this, &TsneAnalysisPlugin::onDistanceMetricPicked);
    connect(&_tsne, &TsneAnalysis::computationStopped, _settings.get(), &TsneSettingsWidget::computationStopped);
    connect(&_tsne, SIGNAL(newEmbedding()), this, SLOT(onNewEmbedding()));
}

void TsneAnalysisPlugin::dataAdded(const QString name)
{
    _settings->dataOptions.addItem(name);
}

void TsneAnalysisPlugin::dataChanged(const QString name)
{
    if (name != _settings->currentData()) {
        return;
    }

    Points& points = _core->requestData<Points>(name);

    _settings->dataChanged(points);
}

void TsneAnalysisPlugin::dataRemoved(const QString name)
{

}

void TsneAnalysisPlugin::selectionChanged(const QString dataName)
{

}


DataTypes TsneAnalysisPlugin::supportedDataTypes() const
{
    DataTypes supportedTypes;
    supportedTypes.append(PointType);
    return supportedTypes;
}

SettingsWidget* const TsneAnalysisPlugin::getSettings()
{
    return _settings.get();
}

void TsneAnalysisPlugin::dataSetPicked(const QString& name)
{
    Points& points = _core->requestData<Points>(name);

    _settings->dataChanged(points);
}

void TsneAnalysisPlugin::onKnnAlgorithmPicked(const int index)
{
    _tsne.setKnnAlgorithm(index);
}

void TsneAnalysisPlugin::onDistanceMetricPicked(const int index)
{
    _tsne.setDistanceMetric(index);
}

void TsneAnalysisPlugin::startComputation()
{
    initializeTsne();

    // Run the computation
    QString setName = _settings->dataOptions.currentText();
    const Points& points = _core->requestData<Points>(setName);

    std::vector<bool> enabledDimensions = _settings->getEnabledDimensions();

    unsigned int numDimensions = count_if(enabledDimensions.begin(), enabledDimensions.end(), [](bool b) { return b; });

    // Create list of data from the enabled dimensions
    std::vector<float> data;

    auto selection = points.indices;

    if (points.isFull()) {
        std::vector<std::uint32_t> all(points.getNumPoints());
        std::iota(std::begin(all), std::end(all), 0);

        selection = all;
    }

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

    _embeddingName = _core->createDerivedData("Points", "Embedding", points.getName());
    Points& embedding = _core->requestData<Points>(_embeddingName);
    
    embedding.setData(nullptr, 0, 2);
    _core->notifyDataAdded(_embeddingName);

    // Compute t-SNE with the given data
    _tsne.initTSNE(data, numDimensions);

    _tsne.start();
}

void TsneAnalysisPlugin::onNewEmbedding() {
    const TsneData& outputData = _tsne.output();
    Points& embedding = _core->requestData<Points>(_embeddingName);
    
    embedding.setData(outputData.getData().data(), outputData.getNumPoints(), 2);

    _core->notifyDataChanged(_embeddingName);
}

void TsneAnalysisPlugin::initializeTsne() {
    // Initialize the tSNE computation with the settings from the settings widget
    _tsne.setIterations(_settings->numIterations.text().toInt());
    _tsne.setPerplexity(_settings->perplexity.text().toInt());
    _tsne.setExaggerationIter(_settings->exaggeration.text().toInt());
    _tsne.setNumTrees(_settings->numTrees.text().toInt());
    _tsne.setNumChecks(_settings->numChecks.text().toInt());
}

void TsneAnalysisPlugin::stopComputation() {
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

AnalysisPlugin* TsneAnalysisPluginFactory::produce()
{
    return new TsneAnalysisPlugin();
}
