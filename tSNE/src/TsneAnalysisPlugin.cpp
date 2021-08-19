#include "TsneAnalysisPlugin.h"

#include "PointData.h"

#include <QtCore>
#include <QtDebug>
#include <QMenu>
#include <QPainter>

Q_PLUGIN_METADATA(IID "nl.tudelft.TsneAnalysisPlugin")

#include <set>

using namespace hdps;
using namespace hdps::gui;

TsneAnalysisPlugin::TsneAnalysisPlugin(const PluginFactory* factory) :
    AnalysisPlugin(factory),
    _tsneAnalysis(),
    _tsneSettingsAction(this),
    _dimensionSelectionAction(this)
{
}

TsneAnalysisPlugin::~TsneAnalysisPlugin(void)
{
    _tsneSettingsAction.getComputationAction().getStopComputationAction().trigger();
}

void TsneAnalysisPlugin::init()
{
    setOutputDatasetName(_core->createDerivedData("tsne_embedding", getInputDatasetName()));

    // Get input/output datasets
    auto& inputDataset = getInputDataset<Points>();
    auto& outputDataset = getOutputDataset<Points>();

    std::vector<float> initialData;

    const auto numEmbeddingDimensions = 2;

    initialData.resize(inputDataset.getNumPoints() * numEmbeddingDimensions);

    outputDataset.setData(initialData.data(), inputDataset.getNumPoints(), numEmbeddingDimensions);

    outputDataset.addAction(_tsneSettingsAction.getGeneralTsneSettingsAction());
    outputDataset.addAction(_tsneSettingsAction.getAdvancedTsneSettingsAction());
    outputDataset.addAction(_dimensionSelectionAction);
    outputDataset.addAction(_tsneSettingsAction.getComputationAction());

    _core->getDataHierarchyItem(outputDataset.getName())->select();

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
        auto& embedding = getOutputDataset<Points>();
        embedding.setData(tsneData.getData().data(), tsneData.getNumPoints(), 2);
        _core->notifyDataChanged(getOutputDatasetName());
    });

    _dimensionSelectionAction.dataChanged(inputDataset);

    connect(&computationAction.getRunningAction(), &ToggleAction::toggled, this, [this, &computationAction, updateComputationAction](bool toggled) {
        _dimensionSelectionAction.setEnabled(!toggled);

        updateComputationAction();
    });

    updateComputationAction();

    registerDataEventByType(PointType, std::bind(&TsneAnalysisPlugin::onDataEvent, this, std::placeholders::_1));

    setTaskName("TSNE");
}

void TsneAnalysisPlugin::onDataEvent(hdps::DataEvent* dataEvent)
{
    if (dataEvent->getType() == EventType::DataChanged)
    {
        auto dataChangedEvent = static_cast<DataChangedEvent*>(dataEvent);

        // If we are not looking at the changed dataset, ignore it
        if (dataChangedEvent->dataSetName != getInputDatasetName())
            return;

        _dimensionSelectionAction.dataChanged(_core->requestData<Points>(dataChangedEvent->dataSetName));
    }
}

QIcon TsneAnalysisPlugin::getIcon() const
{
    return QIcon(":/images/icon.png");
}

void TsneAnalysisPlugin::startComputation()
{
    setTaskRunning();
    setTaskProgress(0.0f);
    setTaskDescription("Preparing data");

    const auto& inputPoints = getInputDataset<Points>();

    // Create list of data from the enabled dimensions
    std::vector<float> data;
    std::vector<unsigned int> indices;

    // Extract the enabled dimensions from the data
    std::vector<bool> enabledDimensions = _dimensionSelectionAction.getEnabledDimensions();

    const auto numEnabledDimensions = count_if(enabledDimensions.begin(), enabledDimensions.end(), [](bool b) { return b; });

    data.resize((inputPoints.isFull() ? inputPoints.getNumPoints() : inputPoints.indices.size()) * numEnabledDimensions);

    for (int i = 0; i < inputPoints.getNumDimensions(); i++)
        if (enabledDimensions[i])
            indices.push_back(i);

    inputPoints.populateDataForDimensions<std::vector<float>, std::vector<unsigned int>>(data, indices);

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

AnalysisPlugin* TsneAnalysisPluginFactory::produce()
{
    return new TsneAnalysisPlugin(this);
}

hdps::DataTypes TsneAnalysisPluginFactory::supportedDataTypes() const
{
    DataTypes supportedTypes;
    supportedTypes.append(PointType);
    return supportedTypes;
}
