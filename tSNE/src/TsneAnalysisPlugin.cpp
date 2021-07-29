#include "TsneAnalysisPlugin.h"
#include "TsneSettingsWidget.h"

#include "PointData.h"

#include <QtCore>
#include <QtDebug>
#include <QMenu>

Q_PLUGIN_METADATA(IID "nl.tudelft.TsneAnalysisPlugin")

#include <set>

using namespace hdps;

TsneAnalysisPlugin::TsneAnalysisPlugin() :
	AnalysisPlugin("tSNE Analysis"),
	_tsneAnalysis(),
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

void TsneAnalysisPlugin::initComputation()
{
	notifyStarted();
	notifyProgressPercentage(0.0f);
	notifyProgressSection("Preparing data");

	const Points& points = _core->requestData<Points>(_inputDatasetName);

    //TsneParameters parameters = _settings->getTsneParameters();
	const auto numDimensions = points.getNumDimensions();

	// Create list of data from the enabled dimensions
	std::vector<float> data;

	auto selection = points.indices;

	// If the dataset is not a subset, use all data points
	if (points.isFull()) {
		std::vector<std::uint32_t> all(points.getNumPoints());
		std::iota(std::begin(all), std::end(all), 0);

		selection = all;
	}

	std::vector<bool> enabledDimensions = _dimensionsSettingsAction.getEnabledDimensions();

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

    /*
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
    */
	//_tsneAnalysis.startComputation(data, numEnabledDimensions);
}

void TsneAnalysisPlugin::startComputation(const bool& restart)
{
	if (_tsneAnalysisDirty)
		initComputation();

	notifyStarted();

	_generalSettingsAction.getRunningAction().setChecked(true);

	//_tsneAnalysis.start();
}

void TsneAnalysisPlugin::stopComputation()
{
    /*
	if (_tsneAnalysis.isRunning())
	{
		// Request interruption of the computation
		_tsneAnalysis.stopGradientDescent();
		_tsneAnalysis.exit();

		// Wait until the thread has terminated (max. 3 seconds)
		if (!_tsneAnalysis.wait(3000))
		{
			qDebug() << "tSNE computation thread did not close in time, terminating...";
			_tsneAnalysis.terminate();
			_tsneAnalysis.wait();

			notifyAborted("Interrupted by user");
		}
		qDebug() << "tSNE computation stopped.";

		notifyAborted("Interrupted by user");
		//notifyProgressSection("");
	}
    */

	notifyAborted("Interrupted by user");
}

AnalysisPlugin* TsneAnalysisPluginFactory::produce()
{
	return new TsneAnalysisPlugin();
}
