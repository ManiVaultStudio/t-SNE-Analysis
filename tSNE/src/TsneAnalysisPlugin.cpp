#include "TsneAnalysisPlugin.h"
#include "TsneSettingsWidget.h"

#include "PointData.h"

#include <QtCore>
#include <QtDebug>
#include <QMenu>

Q_PLUGIN_METADATA(IID "nl.tudelft.TsneAnalysisPlugin")
#ifdef _WIN32
#include <windows.h>
#endif

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
    connect(&_tsne, &TsneAnalysis::finished, _settings.get(), &TsneSettingsWidget::onComputationStopped);
    connect(&_tsne, &TsneAnalysis::embeddingUpdate, this, &TsneAnalysisPlugin::onNewEmbedding);

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

    auto analyses = points.getProperty("Analyses", QVariantList()).toList();

    analyses.push_back(getName());

    points.setProperty("Analyses", analyses);

    _settings->getDimensionSelectionWidget().dataChanged(points);

    _settings->setTitle(QString("%1: %2").arg(getGuiName(), name));
}

void TsneAnalysisPlugin::startComputation()
{
    _settings->setIcon(hdps::Application::getIconFont("FontAwesome").getIcon("play"));
    _settings->setSubtitle("Initializing A-tSNE...");

    qApp->processEvents();

    TsneParameters parameters = _settings->getTsneParameters();

    // Prepare the data
    QString setName = _settings->getCurrentDataItem();
    const Points& points = _core->requestData<Points>(setName);

    unsigned int numDimensions = points.getNumDimensions();

    // Create list of data from the enabled dimensions
    std::vector<float> data;
    std::vector<unsigned int> indices;

    // Extract the enabled dimensions from the data
    std::vector<bool> enabledDimensions = _settings->getDimensionSelectionWidget().getEnabledDimensions();
    unsigned int numEnabledDimensions = count_if(enabledDimensions.begin(), enabledDimensions.end(), [](bool b) { return b; });
    data.resize((points.isFull() ? points.getNumPoints() : points.indices.size()) * numEnabledDimensions);
    for (int i = 0; i < points.getNumDimensions(); i++)
        if (enabledDimensions[i]) indices.push_back(i);

    points.populateDataForDimensions<std::vector<float>, std::vector<unsigned int>>(data, indices);

    // Create new data set for the embedding
    _embeddingName = _core->createDerivedData(_settings->getEmbeddingName(), points.getName());

    Points& embedding = _core->requestData<Points>(_embeddingName);
    embedding.setData(nullptr, 0, 2);
    _core->notifyDataAdded(_embeddingName);

    _tsne.startComputation(parameters, data, numEnabledDimensions);
}

void TsneAnalysisPlugin::onNewEmbedding(const TsneData tsneData) {
    Points& embedding = _core->requestData<Points>(_embeddingName);

    embedding.setData(tsneData.getData().data(), tsneData.getNumPoints(), 2);

    _core->notifyDataChanged(_embeddingName);
}

void TsneAnalysisPlugin::stopComputation() {
    _settings->setIcon(hdps::Application::getIconFont("FontAwesome").getIcon("stop"));
    _settings->setSubtitle("Stopping computation...");

    _tsne.stopComputation();
    qDebug() << "tSNE computation stopped.";
}

QMenu* TsneAnalysisPlugin::contextMenu(const QVariant& context)
{
    auto menu = new QMenu(getGuiName());
    
    auto startComputationAction = new QAction("Start computation");
    auto stopComputationAction = new QAction("Stop computation");

    connect(startComputationAction, &QAction::triggered, [this]() { startComputation(); });
    connect(stopComputationAction, &QAction::triggered, [this]() { stopComputation(); });

    menu->addAction(startComputationAction);
    menu->addAction(stopComputationAction);

    return menu;
}

// =============================================================================
// Factory
// =============================================================================

AnalysisPlugin* TsneAnalysisPluginFactory::produce()
{
    return new TsneAnalysisPlugin();
}