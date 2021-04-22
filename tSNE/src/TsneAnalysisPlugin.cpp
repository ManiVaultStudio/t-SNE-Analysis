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
TsneAnalysisPlugin::TsneAnalysisPlugin() :
    AnalysisPlugin("tSNE Analysis")
{
	QObject::connect(&_tsne, &TsneAnalysis::progressMessage, [this](const QString& message) {
		_settings->setSubtitle(message);
	});
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
    connect(&_tsne, &TsneAnalysis::computationStopped, _settings.get(), &TsneSettingsWidget::onComputationStopped);
    connect(&_tsne, SIGNAL(newEmbedding()), this, SLOT(onNewEmbedding()));

    registerDataEventByType(PointType, std::bind(&TsneAnalysisPlugin::onDataEvent, this, std::placeholders::_1));
}

void TsneAnalysisPlugin::onDataEvent(hdps::DataEvent* dataEvent)
{
    if (dataEvent->getType() == EventType::DataAdded)
        _settings->addDataItem(static_cast<DataAddedEvent*>(dataEvent)->dataSetName);

    if (dataEvent->getType() == EventType::DataRemoved)
        _settings->removeDataItem(static_cast<DataRemovedEvent*>(dataEvent)->dataSetName);

    if (dataEvent->getType() == EventType::DataChanged)
    {
        auto dataChangedEvent = static_cast<DataChangedEvent*>(dataEvent);

        // If we are not looking at the changed dataset, ignore it
        if (dataChangedEvent->dataSetName != _settings->getCurrentDataItem())
            return;

        // Passes changes to the current dataset to the dimension selection widget
        Points& points = _core->requestData<Points>(dataChangedEvent->dataSetName);

        _settings->getDimensionSelectionWidget().dataChanged(points);
    }
}

hdps::gui::SettingsWidget* const TsneAnalysisPlugin::getSettings()
{
    return _settings.get();
}

void TsneAnalysisPlugin::dataSetPicked(const QString& name)
{
    Points& points = _core->requestData<Points>(name);

    _settings->getDimensionSelectionWidget().dataChanged(points);

    _settings->setTitle(QString("%1: %2").arg(getGuiName(), name));
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
    _settings->setIcon(hdps::Application::getIconFont("FontAwesome").getIcon("play"));
    _settings->setSubtitle("Initializing A-tSNE...");

    qApp->processEvents();

    initializeTsne();

    // Prepare the data
    QString setName = _settings->getCurrentDataItem();
    const Points& points = _core->requestData<Points>(setName);

    unsigned int numDimensions = points.getNumDimensions();

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
    unsigned int numEnabledDimensions = count_if(enabledDimensions.begin(), enabledDimensions.end(), [](bool b) { return b; });

    data.reserve(selection.size() * numEnabledDimensions);

    points.visitFromBeginToEnd([&data, &selection, &enabledDimensions, numDimensions](auto beginOfData, auto endOfData)
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

    // Create new data set for the embedding
    _embeddingName = _core->createDerivedData(_settings->getEmbeddingName(), points.getName());

    Points& embedding = _core->requestData<Points>(_embeddingName);
    embedding.setData(nullptr, 0, 2);
    _core->notifyDataAdded(_embeddingName);

    _tsne.initTSNE(data, numEnabledDimensions);

    _tsne.reset();
    _tsne.startComputation();
}

void TsneAnalysisPlugin::continueComputation()
{
    _tsne.startComputation();
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
    _tsne.setExponentialDecayIter(_settings->expDecay.text().toInt());
    _tsne.setNumTrees(_settings->numTrees.text().toInt());
    _tsne.setNumChecks(_settings->numChecks.text().toInt());
}

void TsneAnalysisPlugin::stopComputation() {
	_settings->setIcon(hdps::Application::getIconFont("FontAwesome").getIcon("stop"));
	_settings->setSubtitle("Stopping computation...");

    _tsne.stopComputation();
}

// =============================================================================
// Factory
// =============================================================================

AnalysisPlugin* TsneAnalysisPluginFactory::produce()
{
    return new TsneAnalysisPlugin();
}
