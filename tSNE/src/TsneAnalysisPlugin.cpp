#include "TsneAnalysisPlugin.h"

#include "PointData.h"

#include <QtCore>
#include <QtDebug>
#include <QMenu>

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
    _tsneSettingsAction.getStopComputationAction().trigger();
}

void TsneAnalysisPlugin::init()
{
    _outputDatasetName = _core->createDerivedData(QString("%1_embedding").arg(_inputDatasetName), _inputDatasetName);

    auto& inputDataset  = _core->requestData<Points>(_inputDatasetName);
    auto& outputDataset = _core->requestData<Points>(_outputDatasetName);

    std::vector<float> initialData;

    const auto numEmbeddingDimensions = 2;

    initialData.resize(inputDataset.getNumPoints() * numEmbeddingDimensions);

    outputDataset.setData(initialData.data(), inputDataset.getNumPoints(), numEmbeddingDimensions);
    outputDataset.setParentDatasetName(_inputDatasetName);

    outputDataset.exposeAction(&_tsneSettingsAction);
    outputDataset.exposeAction(&_tsneSettingsAction.getGeneralTsneSettingsAction());
    outputDataset.exposeAction(&_tsneSettingsAction.getAdvancedTsneSettingsAction());
    outputDataset.exposeAction(&_dimensionSelectionAction);

    connect(&_tsneAnalysis, &TsneAnalysis::progressPercentage, this, [this](const float& percentage) {
        notifyProgressPercentage(percentage);
    });

    connect(&_tsneAnalysis, &TsneAnalysis::progressSection, this, [this](const QString& section) {
        notifyProgressSection(section);
    });

    connect(&_tsneAnalysis, &TsneAnalysis::finished, this, [this]() {
        notifyFinished();
        notifyProgressPercentage(0.0f);
        notifyProgressSection("");
        
        _tsneSettingsAction.getRunningAction().setChecked(false);
    });

    connect(&_tsneAnalysis, &TsneAnalysis::finished, this, [this]() {
        notifyFinished();
        notifyProgressPercentage(0.0f);
        notifyProgressSection("");

        _tsneSettingsAction.getRunningAction().setChecked(false);
    });

    connect(&_tsneSettingsAction.getStartComputationAction(), &TriggerAction::triggered, this, [this]() {
        startComputation();
    });

    connect(&_tsneSettingsAction.getContinueComputationAction(), &TriggerAction::triggered, this, [this]() {
        continueComputation();
    });

    connect(&_tsneSettingsAction.getStopComputationAction(), &TriggerAction::triggered, this, [this]() {
        stopComputation();
    });

    connect(&_tsneAnalysis, &TsneAnalysis::embeddingUpdate, this, [this](const TsneData tsneData) {
        auto& embedding = _core->requestData<Points>(_outputDatasetName);
        embedding.setData(tsneData.getData().data(), tsneData.getNumPoints(), 2);
        _core->notifyDataChanged(_outputDatasetName);
    });

    _dimensionSelectionAction.dataChanged(inputDataset);

    connect(&_tsneSettingsAction.getRunningAction(), &ToggleAction::toggled, this, [this](bool toggled) {
        _dimensionSelectionAction.setEnabled(!toggled);
        _tsneSettingsAction.getStartComputationAction().setEnabled(!toggled);
        _tsneSettingsAction.getContinueComputationAction().setEnabled(!toggled && _tsneAnalysis.canContinue());
        _tsneSettingsAction.getStopComputationAction().setEnabled(toggled);
    });

    registerDataEventByType(PointType, std::bind(&TsneAnalysisPlugin::onDataEvent, this, std::placeholders::_1));
}

void TsneAnalysisPlugin::onDataEvent(hdps::DataEvent* dataEvent)
{
    if (dataEvent->getType() == EventType::DataChanged)
    {
        auto dataChangedEvent = static_cast<DataChangedEvent*>(dataEvent);

        // If we are not looking at the changed dataset, ignore it
        if (dataChangedEvent->dataSetName != _inputDatasetName)
            return;

        _dimensionSelectionAction.dataChanged(_core->requestData<Points>(dataChangedEvent->dataSetName));
    }
}

QIcon TsneAnalysisPlugin::getIcon() const
{
    return hdps::Application::getIconFont("FontAwesome").getIcon("table");
}

void TsneAnalysisPlugin::startComputation()
{
    notifyStarted();
    notifyProgressPercentage(0.0f);
    notifyProgressSection("Preparing data");

    const auto& inputPoints = _core->requestData<Points>(_inputDatasetName);

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

    _tsneSettingsAction.getRunningAction().setChecked(true);

    _tsneAnalysis.startComputation(_tsneSettingsAction.getTsneParameters(), data, numEnabledDimensions);
}

void TsneAnalysisPlugin::continueComputation()
{
    notifyStarted();
    notifyProgressPercentage(0.0f);

    _tsneSettingsAction.getRunningAction().setChecked(true);

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
