#include "TsneAnalysisPlugin.h"

#include <PointData/PointData.h>
#include <PointData/DimensionsPickerAction.h>

#include <util/Icon.h>

#include <actions/PluginTriggerAction.h>

#include <QtCore>
#include <QtDebug>
#include <QMenu>
#include <QPainter>

Q_PLUGIN_METADATA(IID "nl.tudelft.TsneAnalysisPlugin")

#include <set>

using namespace mv;
using namespace mv::util;

TsneAnalysisPlugin::TsneAnalysisPlugin(const PluginFactory* factory) :
    AnalysisPlugin(factory),
    _tsneAnalysis(),
    _tsneSettingsAction(this),
    _initializationTask(this, "Preparing TSNE computation")
{
    setObjectName("TSNE");

    _initializationTask.setDescription("All operations prior to TSNE computation");
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

    _initializationTask.setParentTask(&outputDataset->getTask());

    std::vector<float> initialData;

    const auto numEmbeddingDimensions = 2;

    initialData.resize(inputDataset->getNumPoints() * numEmbeddingDimensions);

    outputDataset->setData(initialData.data(), inputDataset->getNumPoints(), numEmbeddingDimensions);

    events().notifyDatasetDataChanged(outputDataset);

    outputDataset->addAction(_tsneSettingsAction.getGeneralTsneSettingsAction());
    outputDataset->addAction(_tsneSettingsAction.getAdvancedTsneSettingsAction());

    auto dimensionsGroupAction = new GroupAction(this, "Dimensions", true);

    dimensionsGroupAction->addAction(&inputDataset->getFullDataset<Points>()->getDimensionsPickerAction());
    dimensionsGroupAction->setText(QString("Input dimensions (%1)").arg(inputDataset->getFullDataset<Points>()->text()));
    dimensionsGroupAction->setShowLabels(false);

    outputDataset->addAction(*dimensionsGroupAction);

    outputDataset->getDataHierarchyItem().select();

    auto& computationAction = _tsneSettingsAction.getComputationAction();

    const auto updateComputationAction = [this, &computationAction]() {
        const auto isRunning = computationAction.getRunningAction().isChecked();

        computationAction.getStartComputationAction().setEnabled(!isRunning);
        computationAction.getContinueComputationAction().setEnabled(!isRunning && _tsneAnalysis.canContinue());
        computationAction.getStopComputationAction().setEnabled(isRunning);
    };

    connect(&_tsneAnalysis, &TsneAnalysis::finished, this, [this, &computationAction]() {
        computationAction.getRunningAction().setChecked(false);

        _tsneSettingsAction.getGeneralTsneSettingsAction().setReadOnly(false);
        _tsneSettingsAction.getAdvancedTsneSettingsAction().setReadOnly(false);
    });

    connect(&_tsneAnalysis, &TsneAnalysis::aborted, this, [this, &computationAction, updateComputationAction]() {
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
        qApp->processEvents();

        stopComputation();
    });

    connect(&_tsneAnalysis, &TsneAnalysis::embeddingUpdate, this, [this](const TsneData tsneData) {

        // Update the output points dataset with new data from the TSNE analysis
        getOutputDataset<Points>()->setData(tsneData.getData().data(), tsneData.getNumPoints(), 2);

        _tsneSettingsAction.getGeneralTsneSettingsAction().getNumberOfComputatedIterationsAction().setValue(_tsneAnalysis.getNumIterations() - 1);

        QCoreApplication::processEvents();

        // Notify others that the embedding data changed
        events().notifyDatasetDataChanged(getOutputDataset());
    });

    connect(&computationAction.getRunningAction(), &ToggleAction::toggled, this, [this, &computationAction, updateComputationAction](bool toggled) {
        getInputDataset<Points>()->getDimensionsPickerAction().setEnabled(!toggled);
        updateComputationAction();
    });

    updateComputationAction();

    auto& datasetTask = getOutputDataset()->getTask();

    datasetTask.setName("TSNE Computation");
    datasetTask.setConfigurationFlag(Task::ConfigurationFlag::OverrideAggregateStatus);
 
    _tsneAnalysis.setTask(&datasetTask);

    //_tsneSettingsAction.loadDefault();
}

void TsneAnalysisPlugin::startComputation()
{
    getOutputDataset()->getTask().setRunning();

    _initializationTask.setEnabled(true);
    _initializationTask.setRunning();

    auto inputPoints = getInputDataset<Points>();

    // Create list of data from the enabled dimensions
    std::vector<float> data;
    std::vector<unsigned int> indices;

    // Extract the enabled dimensions from the data
    std::vector<bool> enabledDimensions = getInputDataset<Points>()->getDimensionsPickerAction().getEnabledDimensions();

    const auto numEnabledDimensions = count_if(enabledDimensions.begin(), enabledDimensions.end(), [](bool b) { return b; });

    _tsneSettingsAction.getGeneralTsneSettingsAction().getNumberOfComputatedIterationsAction().reset();

    data.resize((inputPoints->isFull() ? inputPoints->getNumPoints() : inputPoints->indices.size()) * numEnabledDimensions);

    for (int i = 0; i < inputPoints->getNumDimensions(); i++)
        if (enabledDimensions[i])
            indices.push_back(i);

    inputPoints->populateDataForDimensions<std::vector<float>, std::vector<unsigned int>>(data, indices);

    _tsneSettingsAction.getComputationAction().getRunningAction().setChecked(true);
    
    _initializationTask.setFinished();

    _tsneAnalysis.startComputation(_tsneSettingsAction.getTsneParameters(), data, numEnabledDimensions);
}

void TsneAnalysisPlugin::continueComputation()
{
    getOutputDataset()->getTask().setRunning();

    _initializationTask.setEnabled(false);

    _tsneSettingsAction.getComputationAction().getRunningAction().setChecked(true);

    _tsneAnalysis.continueComputation(_tsneSettingsAction.getTsneParameters().getNumIterations());
}

void TsneAnalysisPlugin::stopComputation()
{
    _tsneAnalysis.stopComputation();
}

QIcon TsneAnalysisPluginFactory::getIcon(const QColor& color /*= Qt::black*/) const
{
    return createPluginIcon("TSNE", color);
}

AnalysisPlugin* TsneAnalysisPluginFactory::produce()
{
    return new TsneAnalysisPlugin(this);
}

PluginTriggerActions TsneAnalysisPluginFactory::getPluginTriggerActions(const mv::Datasets& datasets) const
{
    PluginTriggerActions pluginTriggerActions;

    const auto getPluginInstance = [this](const Dataset<Points>& dataset) -> TsneAnalysisPlugin* {
        return dynamic_cast<TsneAnalysisPlugin*>(plugins().requestPlugin(getKind(), { dataset }));
    };

    if (PluginFactory::areAllDatasetsOfTheSameType(datasets, PointType)) {
        if (datasets.count() >= 1) {
            auto pluginTriggerAction = new PluginTriggerAction(const_cast<TsneAnalysisPluginFactory*>(this), this, "TSNE", "Perform TSNE analysis on selected datasets", getIcon(), [this, getPluginInstance, datasets](PluginTriggerAction& pluginTriggerAction) -> void {
                for (auto dataset : datasets)
                    getPluginInstance(dataset);
            });

            pluginTriggerActions << pluginTriggerAction;
        }

        if (datasets.count() >= 2) {
            auto pluginTriggerAction = new PluginTriggerAction(const_cast<TsneAnalysisPluginFactory*>(this), this, "Group/TSNE", "Group datasets and perform TSNE analysis on it", getIcon(), [this, getPluginInstance, datasets](PluginTriggerAction& pluginTriggerAction) -> void {
                getPluginInstance(Application::core()->groupDatasets(datasets));
            });

            pluginTriggerActions << pluginTriggerAction;
        }
    }

    return pluginTriggerActions;
}

