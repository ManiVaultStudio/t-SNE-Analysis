#include "TsneAnalysisPlugin.h"

#include <PointData.h>

#include <util/Icon.h>

#include <QtCore>
#include <QtDebug>
#include <QMenu>
#include <QPainter>

Q_PLUGIN_METADATA(IID "nl.tudelft.TsneAnalysisPlugin")

#include <set>

using namespace hdps;
using namespace hdps::util;

TsneAnalysisPlugin::TsneAnalysisPlugin(const PluginFactory* factory) :
    AnalysisPlugin(factory),
    _tsneAnalysis(),
    _tsneSettingsAction(this)
{
    setObjectName("TSNE");
}

TsneAnalysisPlugin::~TsneAnalysisPlugin(void)
{
    _tsneSettingsAction.getComputationAction().getStopComputationAction().trigger();
}

void TsneAnalysisPlugin::init()
{
    setOutputDataset(_core->createDerivedDataset("TSNE Embedding", getInputDataset(), getInputDataset()));

    // Get input/output datasets
    auto inputDataset  = getInputDataset<Points>();
    auto outputDataset = getOutputDataset<Points>();

    std::vector<float> initialData;

    const auto numEmbeddingDimensions = 2;

    initialData.resize(inputDataset->getNumPoints() * numEmbeddingDimensions);

    outputDataset->setData(initialData.data(), inputDataset->getNumPoints(), numEmbeddingDimensions);

    outputDataset->addAction(_tsneSettingsAction.getGeneralTsneSettingsAction());
    outputDataset->addAction(_tsneSettingsAction.getAdvancedTsneSettingsAction());
    outputDataset->addAction(_tsneSettingsAction.getDimensionSelectionAction());

    outputDataset->getDataHierarchyItem().select();

    auto& computationAction = _tsneSettingsAction.getComputationAction();

    const auto updateComputationAction = [this, &computationAction]() {
        const auto isRunning = computationAction.getRunningAction().isChecked();

        computationAction.getStartComputationAction().setEnabled(!isRunning);
        computationAction.getContinueComputationAction().setEnabled(!isRunning && _tsneAnalysis.canContinue());
        computationAction.getStopComputationAction().setEnabled(isRunning);
    };

    connect(&_tsneAnalysis, &TsneAnalysis::progressPercentage, this, [this](const float& percentage) {
        if (getTaskStatus() == DataHierarchyItem::TaskStatus::Aborted)
            return;

        setTaskProgress(percentage);
    });

    connect(&_tsneAnalysis, &TsneAnalysis::progressSection, this, [this](const QString& section) {
        if (getTaskStatus() == DataHierarchyItem::TaskStatus::Aborted)
            return;

        setTaskDescription(section);
    });

    connect(&_tsneAnalysis, &TsneAnalysis::finished, this, [this, &computationAction]() {
        setTaskFinished();
        
        computationAction.getRunningAction().setChecked(false);

        _tsneSettingsAction.getGeneralTsneSettingsAction().setReadOnly(false);
        _tsneSettingsAction.getAdvancedTsneSettingsAction().setReadOnly(false);
    });

    connect(&_tsneAnalysis, &TsneAnalysis::aborted, this, [this, &computationAction, updateComputationAction]() {
        setTaskAborted();

        updateComputationAction();

        computationAction.getRunningAction().setChecked(false);

        _tsneSettingsAction.getGeneralTsneSettingsAction().setReadOnly(false);
        _tsneSettingsAction.getAdvancedTsneSettingsAction().setReadOnly(false);
    });

    connect(&computationAction.getStartComputationAction(), &TriggerAction::triggered, this, [this, &computationAction]() {
        _tsneSettingsAction.getGeneralTsneSettingsAction().setReadOnly(true);
        _tsneSettingsAction.getAdvancedTsneSettingsAction().setReadOnly(true);

        startComputation();
    });

    connect(&computationAction.getContinueComputationAction(), &TriggerAction::triggered, this, [this]() {
        _tsneSettingsAction.getGeneralTsneSettingsAction().setReadOnly(true);
        _tsneSettingsAction.getAdvancedTsneSettingsAction().setReadOnly(true);

        continueComputation();
    });

    connect(&computationAction.getStopComputationAction(), &TriggerAction::triggered, this, [this]() {
        setTaskDescription("Aborting TSNE");

        qApp->processEvents();

        stopComputation();
    });

    connect(&_tsneAnalysis, &TsneAnalysis::embeddingUpdate, this, [this](const TsneData tsneData) {

        // Update the output points dataset with new data from the TSNE analysis
        getOutputDataset<Points>()->setData(tsneData.getData().data(), tsneData.getNumPoints(), 2);

        _tsneSettingsAction.getGeneralTsneSettingsAction().getNumberOfComputatedIterationsAction().setValue(_tsneAnalysis.getNumIterations() - 1);

        QCoreApplication::processEvents();

        // Notify others that the embedding data changed
        _core->notifyDatasetChanged(getOutputDataset());
    });

    _tsneSettingsAction.getDimensionSelectionAction().getPickerAction().setPointsDataset(inputDataset);

    connect(&computationAction.getRunningAction(), &ToggleAction::toggled, this, [this, &computationAction, updateComputationAction](bool toggled) {
        _tsneSettingsAction.getDimensionSelectionAction().setEnabled(!toggled);

        updateComputationAction();
    });

    updateComputationAction();

    _eventListener.setEventCore(Application::core());
    _eventListener.addSupportedEventType(static_cast<std::uint32_t>(EventType::DataChanged));
    _eventListener.registerDataEventByType(PointType, std::bind(&TsneAnalysisPlugin::onDataEvent, this, std::placeholders::_1));

    setTaskName("TSNE");
 
    //_tsneSettingsAction.loadDefault();
}

void TsneAnalysisPlugin::onDataEvent(hdps::DataEvent* dataEvent)
{
    if (dataEvent->getType() == EventType::DataChanged)
    {
        if (dataEvent->getDataset() == getInputDataset())
            _tsneSettingsAction.getDimensionSelectionAction().getPickerAction().setPointsDataset(dataEvent->getDataset<Points>());
    }
}

void TsneAnalysisPlugin::startComputation()
{
    setTaskRunning();
    setTaskProgress(0.0f);
    setTaskDescription("Preparing data");

    _tsneSettingsAction.getGeneralTsneSettingsAction().getNumberOfComputatedIterationsAction().reset();

    auto inputPoints = getInputDataset<Points>();

    // Create list of data from the enabled dimensions
    std::vector<float> data;
    std::vector<unsigned int> indices;

    // Extract the enabled dimensions from the data
    std::vector<bool> enabledDimensions = _tsneSettingsAction.getDimensionSelectionAction().getPickerAction().getEnabledDimensions();

    const auto numEnabledDimensions = count_if(enabledDimensions.begin(), enabledDimensions.end(), [](bool b) { return b; });

    data.resize((inputPoints->isFull() ? inputPoints->getNumPoints() : inputPoints->indices.size()) * numEnabledDimensions);

    for (int i = 0; i < inputPoints->getNumDimensions(); i++)
        if (enabledDimensions[i])
            indices.push_back(i);

    inputPoints->populateDataForDimensions<std::vector<float>, std::vector<unsigned int>>(data, indices);

    _tsneSettingsAction.getComputationAction().getRunningAction().setChecked(true);

    _tsneAnalysis.startComputation(_tsneSettingsAction.getTsneParameters(), data, numEnabledDimensions);
}

void TsneAnalysisPlugin::continueComputation()
{
    setTaskRunning();
    setTaskProgress(0.0f);

    _tsneSettingsAction.getComputationAction().getRunningAction().setChecked(true);

    _tsneAnalysis.continueComputation(_tsneSettingsAction.getTsneParameters().getNumIterations());
}

void TsneAnalysisPlugin::stopComputation()
{
    _tsneAnalysis.stopComputation();
}

QIcon TsneAnalysisPluginFactory::getIcon(const QColor& color /*= Qt::black*/) const
{
    return createPluginIcon("TSNE");
}

AnalysisPlugin* TsneAnalysisPluginFactory::produce()
{
    return new TsneAnalysisPlugin(this);
}

QList<TriggerAction*> TsneAnalysisPluginFactory::getProducers(const hdps::Datasets& datasets) const
{
    QList<TriggerAction*> producerActions;

    const auto getPluginInstance = [this]() -> AnalysisPlugin* {
        return dynamic_cast<AnalysisPlugin*>(Application::core()->requestPlugin(getKind()));
    };

    if (PluginFactory::areAllDatasetsOfTheSameType(datasets, "Points")) {
        if (datasets.count() >= 1 && datasets.first()->getDataType().getTypeString() == "Points") {
            auto producerAction = createProducerAction("TSNE", "Perform TSNE analysis on dataset");

            connect(producerAction, &QAction::triggered, [this, getPluginInstance, datasets]() -> void {
                for (auto dataset : datasets) {
                    auto pluginInstance = getPluginInstance();

                    pluginInstance->setInputDataset({ dataset });

                    dataset->setAnalysis(pluginInstance);
                }
                });

            producerActions << producerAction;
        }
    }

    return producerActions;
}

