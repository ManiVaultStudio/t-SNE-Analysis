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
    _tsneParameters(),
	_tsneAnalysisDirty(true),
	_generalSettingsAction(this),
	_advancedSettingsAction(this),
	_dimensionsSettingsAction(this)
{
}

TsneAnalysisPlugin::~TsneAnalysisPlugin(void)
{
	_generalSettingsAction.getStopComputationAction().trigger();
}

void TsneAnalysisPlugin::init()
{
	_outputDatasetName = _core->addData("Points", QString("%1_embedding").arg(_inputDatasetName));

	auto& inputDataset	= _core->requestData<Points>(_inputDatasetName);
	auto& outputDataset = _core->requestData<Points>(_outputDatasetName);

	std::vector<float> initialData;

	initialData.resize(inputDataset.getNumPoints() * inputDataset.getNumDimensions());

	outputDataset.setData(initialData, 2);
	outputDataset.setParentDatasetName(_inputDatasetName);

	outputDataset.exposeAction(&_generalSettingsAction);
	outputDataset.exposeAction(&_advancedSettingsAction);
	outputDataset.exposeAction(&_dimensionsSettingsAction);

	connect(&_tsneAnalysis, &TsneAnalysis::progressPercentage, this, [this](const float& percentage) {
		notifyProgressPercentage(percentage);

		if (percentage == 1.0f) {
			_generalSettingsAction.getRunningAction().setChecked(false);
			notifyFinished();
		}
	});

	connect(&_tsneAnalysis, &TsneAnalysis::progressSection, this, [this](const QString& section) {
		notifyProgressSection(section);
	});

	connect(&_tsneAnalysis, &TsneAnalysis::finished, this, [this]() {
		getGeneralSettingsAction().getRunningAction().setChecked(false);
		getGeneralSettingsAction().getRunningAction().setText("");
		getGeneralSettingsAction().getRunningAction().setIcon(hdps::Application::getIconFont("FontAwesome").getIcon("play"));
		notifyFinished();
		notifyProgressSection("");
	});

	connect(&_generalSettingsAction.getStartComputationAction(), &TriggerAction::triggered, this, [this]() {
		startComputation(true);
	});

	connect(&_generalSettingsAction.getContinueComputationAction(), &TriggerAction::triggered, this, [this]() {
		startComputation(false);
	});

	connect(&_generalSettingsAction.getStopComputationAction(), &TriggerAction::triggered, this, [this]() {
		stopComputation();
	});

	connect(&_tsneAnalysis, &TsneAnalysis::embeddingUpdate, this, [this](const TsneData tsneData) {
		auto& embedding = _core->requestData<Points>(_outputDatasetName);
		embedding.setData(tsneData.getData().data(), tsneData.getNumPoints(), 2);
		_core->notifyDataChanged(_outputDatasetName);
	});

	_dimensionsSettingsAction.dataChanged(inputDataset);

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

		_dimensionsSettingsAction.dataChanged(_core->requestData<Points>(dataChangedEvent->dataSetName));

		_tsneAnalysisDirty = true;
	}
}

/*
hdps::DataTypes TsneAnalysisPlugin::supportedDataTypes() const
{
    DataTypes supportedTypes;
    supportedTypes.append(PointType);
    return supportedTypes;
}
*/

QIcon TsneAnalysisPlugin::getIcon() const
{
	return hdps::Application::getIconFont("FontAwesome").getIcon("table");
}

void TsneAnalysisPlugin::startComputation(const bool& restart)
{
    notifyStarted();
    notifyProgressPercentage(0.0f);
    notifyProgressSection("Preparing data");

    const auto& inputPoints = _core->requestData<Points>(_inputDatasetName);

    // Create list of data from the enabled dimensions
    std::vector<float> data;
    std::vector<unsigned int> indices;

    // Extract the enabled dimensions from the data
    std::vector<bool> enabledDimensions = _dimensionsSettingsAction.getEnabledDimensions();

    const auto numEnabledDimensions = count_if(enabledDimensions.begin(), enabledDimensions.end(), [](bool b) { return b; });

    data.resize((inputPoints.isFull() ? inputPoints.getNumPoints() : inputPoints.indices.size()) * numEnabledDimensions);

    for (int i = 0; i < inputPoints.getNumDimensions(); i++)
        if (enabledDimensions[i])
            indices.push_back(i);

    inputPoints.populateDataForDimensions<std::vector<float>, std::vector<unsigned int>>(data, indices);

    _generalSettingsAction.getRunningAction().setChecked(true);

    _tsneAnalysis.startComputation(_tsneParameters, data, numEnabledDimensions);
}

void TsneAnalysisPlugin::stopComputation()
{
    _tsneAnalysis.stopComputation();
    emit notifyAborted("Interrupted by user");
}

TsneParameters& TsneAnalysisPlugin::getTsneParameters()
{
    return _tsneParameters;
}

AnalysisPlugin* TsneAnalysisPluginFactory::produce()
{
	return new TsneAnalysisPlugin();
}
