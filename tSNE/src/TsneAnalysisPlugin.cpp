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

TsneAnalysisPlugin::TsneAnalysisPlugin() :
    AnalysisPlugin("tSNE Analysis"),
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
    setOutputDatasetName(_core->createDerivedData(QString("%1_tsne_embedding").arg(getInputDatasetName()), getInputDatasetName()));

    // Get input/output datasets
    auto& inputDataset = getInputDataset<Points>();
    auto& outputDataset = getOutputDataset<Points>();

    std::vector<float> initialData;

    const auto numEmbeddingDimensions = 2;

    initialData.resize(inputDataset.getNumPoints() * numEmbeddingDimensions);

    outputDataset.setData(initialData.data(), inputDataset.getNumPoints(), numEmbeddingDimensions);

    outputDataset.exposeAction(&_tsneSettingsAction.getGeneralTsneSettingsAction());
    outputDataset.exposeAction(&_tsneSettingsAction.getAdvancedTsneSettingsAction());
    outputDataset.exposeAction(&_dimensionSelectionAction);

    _core->getDataHierarchyItem(outputDataset.getName())->select();

    auto& computationAction = _tsneSettingsAction.getComputationAction();

    connect(&_tsneAnalysis, &TsneAnalysis::progressPercentage, this, [this](const float& percentage) {
        notifyProgressPercentage(percentage);
    });

    connect(&_tsneAnalysis, &TsneAnalysis::progressSection, this, [this](const QString& section) {
        notifyProgressSection(section);
    });

    connect(&_tsneAnalysis, &TsneAnalysis::finished, this, [this, &computationAction]() {
        notifyFinished();
        notifyProgressPercentage(0.0f);
        notifyProgressSection("");
        
        computationAction.getRunningAction().setChecked(false);

        _tsneSettingsAction.getGeneralTsneSettingsAction().setReadOnly(false);
        _tsneSettingsAction.getAdvancedTsneSettingsAction().setReadOnly(false);
    });

    connect(&_tsneAnalysis, &TsneAnalysis::stopped, this, [this, &computationAction]() {
        notifyFinished();
        notifyProgressPercentage(0.0f);
        notifyProgressSection("");

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
        stopComputation();
    });

    connect(&_tsneAnalysis, &TsneAnalysis::embeddingUpdate, this, [this](const TsneData tsneData) {
        auto& embedding = getOutputDataset<Points>();
        embedding.setData(tsneData.getData().data(), tsneData.getNumPoints(), 2);
        _core->notifyDataChanged(getOutputDatasetName());
    });

    _dimensionSelectionAction.dataChanged(inputDataset);

    connect(&computationAction.getRunningAction(), &ToggleAction::toggled, this, [this, &computationAction](bool toggled) {
        _dimensionSelectionAction.setEnabled(!toggled);

        computationAction.getStartComputationAction().setEnabled(!toggled);
        computationAction.getContinueComputationAction().setEnabled(!toggled && _tsneAnalysis.canContinue());
        computationAction.getStopComputationAction().setEnabled(toggled);
    });

    registerDataEventByType(PointType, std::bind(&TsneAnalysisPlugin::onDataEvent, this, std::placeholders::_1));
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
    notifyStarted();
    notifyProgressPercentage(0.0f);
    notifyProgressSection("Preparing data");

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
    notifyStarted();
    notifyProgressPercentage(0.0f);

    _tsneSettingsAction.getComputationAction().getRunningAction().setChecked(true);

    _tsneAnalysis.continueComputation(_tsneSettingsAction.getTsneParameters().getNumIterations());
}

void TsneAnalysisPlugin::stopComputation()
{
    _tsneAnalysis.stopComputation();
    emit notifyAborted("Interrupted by user");
}

AnalysisPlugin* TsneAnalysisPluginFactory::produce()
{
    return new TsneAnalysisPlugin();
}

hdps::DataTypes TsneAnalysisPluginFactory::supportedDataTypes() const
{
    DataTypes supportedTypes;
    supportedTypes.append(PointType);
    return supportedTypes;
}
