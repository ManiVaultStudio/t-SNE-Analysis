#include "TsneAnalysisPlugin.h"
#include "TsneSettingsWidget.h"

#include "PointData.h"

#include <QtCore>
#include <QtDebug>

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
	AnalysisPlugin("tSNE Analysis"),
	_tsne(),
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
	_outputDatasetName = _core->addData("Points", "Embedding");

	DataSet& outputDataset = _core->requestData<Points>(_outputDatasetName);

	outputDataset.setParentDatasetName(_inputDatasetName);

	outputDataset.exposeAction(&_generalSettingsAction);
	outputDataset.exposeAction(&_advancedSettingsAction);
	outputDataset.exposeAction(&_dimensionsSettingsAction);

	/*
	const TsneData& outputData = _tsne.output();
	Points& embedding = _core->requestData<Points>(_embeddingName);

	embedding.setData(outputData.getData().data(), outputData.getNumPoints(), 2);

	_core->notifyDataChanged(_embeddingName);
	*/

	connect(&_tsne, &TsneAnalysis::progressPercentage, this, [this](const float& percentage) {
		notifyProgressPercentage(percentage);
	});

	connect(&_tsne, &TsneAnalysis::progressSection, this, [this](const QString& section) {
		notifyProgressSection(section);
	});

	connect(&_generalSettingsAction.getStartComputationAction(), &TriggerAction::triggered, this, [this]() {

		qApp->processEvents();

		notifyStarted();
		notifyProgressSection("Preparing data");

		const Points& points = _core->requestData<Points>(_inputDatasetName);

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

		// Extract the enabled dimensions from the data
		//std::vector<bool> enabledDimensions = _settings->getDimensionSelectionWidget().getEnabledDimensions();
		std::vector<bool> enabledDimensions;

		enabledDimensions.resize(numDimensions);

		std::fill(enabledDimensions.begin(), enabledDimensions.end(), true);

		unsigned int numEnabledDimensions = count_if(enabledDimensions.begin(), enabledDimensions.end(), [](bool b) { return b; });

		data.resize(selection.size() * numEnabledDimensions);
		//data.reserve(selection.size() * numEnabledDimensions);
		/*
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
		*/

		notifyProgressSection("Initializing");

		_tsne.initTSNE(data, numEnabledDimensions);
		_tsne.start();
		
	});

	connect(&_generalSettingsAction.getStopComputationAction(), &TriggerAction::triggered, this, [this]() {
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

			notifyAborted("Interrupted by user");
		}
	});

	/*
	connect(_settings.get(), &TsneSettingsWidget::knnAlgorithmPicked, this, &TsneAnalysisPlugin::onKnnAlgorithmPicked);
	connect(_settings.get(), &TsneSettingsWidget::distanceMetricPicked, this, &TsneAnalysisPlugin::onDistanceMetricPicked);
	connect(&_tsne, SIGNAL(newEmbedding()), this, SLOT(onNewEmbedding()));

	registerDataEventByType(PointType, std::bind(&TsneAnalysisPlugin::onDataEvent, this, std::placeholders::_1));
	*/
}

void TsneAnalysisPlugin::onDataEvent(hdps::DataEvent* dataEvent)
{
	/*
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
	*/
}

QIcon TsneAnalysisPlugin::getIcon() const
{
	return hdps::Application::getIconFont("FontAwesome").getIcon("table");
}

/*
void TsneAnalysisPlugin::dataSetPicked(const QString& name)
{
	Points& points = _core->requestData<Points>(name);

	_settings->getDimensionSelectionWidget().dataChanged(points);

	_settings->setTitle(QString("%1: %2").arg(getGuiName(), name));keb
}

*/

AnalysisPlugin* TsneAnalysisPluginFactory::produce()
{
	return new TsneAnalysisPlugin();
}
