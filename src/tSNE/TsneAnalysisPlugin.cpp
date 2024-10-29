#include "TsneAnalysisPlugin.h"

#include "TsneSettingsAction.h"

#include <PointData/DimensionsPickerAction.h>
#include <PointData/InfoAction.h>
#include <PointData/PointData.h>

#include <event/Event.h>
#include <util/Icon.h>

#include <actions/PluginTriggerAction.h>

#include "hdi/data/io.h"
#include "hdi/dimensionality_reduction/hd_joint_probability_generator.h"

#include <fstream>
#include <iostream>

Q_PLUGIN_METADATA(IID "nl.tudelft.TsneAnalysisPlugin")

using namespace mv;
using namespace mv::util;

TsneAnalysisPlugin::TsneAnalysisPlugin(const PluginFactory* factory) :
    AnalysisPlugin(factory),
    _tsneAnalysis(),
    _tsneSettingsAction(nullptr),
    _dataPreparationTask(this, "Prepare data"),
    _probDistMatrix()
{
    setObjectName("TSNE");

    _dataPreparationTask.setDescription("All operations prior to TSNE computation");
}

TsneAnalysisPlugin::~TsneAnalysisPlugin(void)
{
    _tsneSettingsAction->getComputationAction().getStopComputationAction().trigger();
}

void TsneAnalysisPlugin::init()
{
    // Get input/output datasets
    auto inputDataset = getInputDataset<Points>();

    // Create derived dataset for embedding
    if (!outputDataInit())
    {
        auto derivedData = mv::data().createDerivedDataset("TSNE Embedding", getInputDataset(), getInputDataset());
        auto newOutput = Dataset<Points>(derivedData.get<Points>());
        setOutputDataset(newOutput);

        const size_t numEmbeddingDimensions = 2;
        std::vector<float> initialData;
        initialData.resize(numEmbeddingDimensions * inputDataset->getNumPoints());

        newOutput->setData(initialData.data(), inputDataset->getNumPoints(), numEmbeddingDimensions);
        events().notifyDatasetDataChanged(newOutput);
    }

    auto outputDataset = getOutputDataset<Points>();

    _tsneSettingsAction = new TsneSettingsAction(this, inputDataset->getNumPoints());

    _dataPreparationTask.setParentTask(&outputDataset->getTask());

    // Manage UI elements attached to output data set
    outputDataset->getDataHierarchyItem().select(true);
    outputDataset->_infoAction->collapse();

    outputDataset->addAction(_tsneSettingsAction->getGeneralTsneSettingsAction());
    outputDataset->addAction(_tsneSettingsAction->getInitalEmbeddingSettingsAction());
    outputDataset->addAction(_tsneSettingsAction->getGradientDescentSettingsAction());
    outputDataset->addAction(_tsneSettingsAction->getKnnSettingsAction());

    auto dimensionsGroupAction = new GroupAction(this, "Dimensions", true);

    dimensionsGroupAction->addAction(&inputDataset->getFullDataset<Points>()->getDimensionsPickerAction());
    dimensionsGroupAction->setText(QString("Input dimensions (%1)").arg(inputDataset->getFullDataset<Points>()->text()));
    dimensionsGroupAction->setShowLabels(false);

    outputDataset->addAction(*dimensionsGroupAction);

    // update settings that depend on number of data points
    _tsneSettingsAction->getGradientDescentSettingsAction().getExaggerationFactorAction().setValue(4.f + inputDataset->getNumPoints() / 60000.0f);

    auto& computationAction = _tsneSettingsAction->getComputationAction();

    const auto updateComputationAction = [this, &computationAction]() {
        const auto isRunning = computationAction.getRunningAction().isChecked();

        computationAction.getStartComputationAction().setEnabled(!isRunning);
        computationAction.getContinueComputationAction().setEnabled(!isRunning && _tsneAnalysis.canContinue());
        computationAction.getStopComputationAction().setEnabled(isRunning);
    };

    auto changeSettingsReadOnly = [this](bool readonly) -> void {
        _tsneSettingsAction->getGeneralTsneSettingsAction().setReadOnly(readonly);
        _tsneSettingsAction->getInitalEmbeddingSettingsAction().setReadOnly(readonly);
        _tsneSettingsAction->getGradientDescentSettingsAction().setReadOnly(readonly);
        _tsneSettingsAction->getKnnSettingsAction().setReadOnly(readonly);
    };

    connect(&_tsneAnalysis, &TsneAnalysis::finished, this, [this, &computationAction, changeSettingsReadOnly]() {
        computationAction.getRunningAction().setChecked(false);

        changeSettingsReadOnly(false);
    });

    connect(&_tsneAnalysis, &TsneAnalysis::aborted, this, [this, &computationAction, updateComputationAction, changeSettingsReadOnly]() {
        updateComputationAction();

        computationAction.getRunningAction().setChecked(false);

        changeSettingsReadOnly(false);    
    });

    connect(&computationAction.getStartComputationAction(), &TriggerAction::triggered, this, [this, &computationAction, changeSettingsReadOnly]() {
        changeSettingsReadOnly(true);

        if(_tsneSettingsAction->getGeneralTsneSettingsAction().getReinitAction().isChecked())
            reinitializeComputation();
        else
            startComputation();

        _tsneSettingsAction->getGeneralTsneSettingsAction().getReinitAction().setCheckable(true);   // only enable re-init after first computation
    });

    connect(&computationAction.getContinueComputationAction(), &TriggerAction::triggered, this, [this, changeSettingsReadOnly]() {
        changeSettingsReadOnly(true);

        continueComputation();
    });

    connect(&computationAction.getStopComputationAction(), &TriggerAction::triggered, this, [this]() {
        qApp->processEvents();

        stopComputation();
    });

    connect(&_tsneAnalysis, &TsneAnalysis::embeddingUpdate, this, [this](const TsneData tsneData) {

        // Update the output points dataset with new data from the TSNE analysis
        getOutputDataset<Points>()->setData(tsneData.getData().data(), tsneData.getNumPoints(), 2);

        _tsneSettingsAction->getGeneralTsneSettingsAction().getNumberOfComputatedIterationsAction().setValue(_tsneAnalysis.getNumIterations() - 1);

        // Notify others that the embedding data changed
        events().notifyDatasetDataChanged(getOutputDataset());
    });

    connect(&computationAction.getRunningAction(), &ToggleAction::toggled, this, [this, &computationAction, updateComputationAction](bool toggled) {
        getInputDataset<Points>()->getDimensionsPickerAction().setEnabled(!toggled);
        updateComputationAction();
    });

    updateComputationAction();

    auto& datasetTask = outputDataset->getTask();

    datasetTask.setName("Compute TSNE");
    datasetTask.setConfigurationFlag(Task::ConfigurationFlag::OverrideAggregateStatus);
 
    _tsneAnalysis.setTask(&datasetTask);

    _eventListener.addSupportedEventType(static_cast<std::uint32_t>(EventType::DatasetDataChanged));
    _eventListener.registerDataEvent([this](DatasetEvent* dataEvent) {
        const auto& dataset = dataEvent->getDataset();

        if (dataset->getDataType() != PointType)
            return;

        _tsneSettingsAction->getInitalEmbeddingSettingsAction().updateDatasetPicker();

        }); 
}

void TsneAnalysisPlugin::startComputation()
{
    getOutputDataset()->getTask().setRunning();

    _dataPreparationTask.setEnabled(true);
    _dataPreparationTask.setRunning();

    auto inputPoints = getInputDataset<Points>();

    // Create list of data from the enabled dimensions
    std::vector<float> data;
    std::vector<unsigned int> indices;

    // Extract the enabled dimensions from the data
    std::vector<bool> enabledDimensions = getInputDataset<Points>()->getDimensionsPickerAction().getEnabledDimensions();

    const auto numEnabledDimensions = count_if(enabledDimensions.begin(), enabledDimensions.end(), [](bool b) { return b; });

    _tsneSettingsAction->getGeneralTsneSettingsAction().getNumberOfComputatedIterationsAction().reset();

    const auto numPoints = inputPoints->isFull() ? inputPoints->getNumPoints() : inputPoints->indices.size();
    data.resize(numPoints * numEnabledDimensions);

    for (int i = 0; i < inputPoints->getNumDimensions(); i++)
        if (enabledDimensions[i])
            indices.push_back(i);

    inputPoints->populateDataForDimensions<std::vector<float>, std::vector<unsigned int>>(data, indices);

    _tsneSettingsAction->getComputationAction().getRunningAction().setChecked(true);

    // Init embedding: random or set from other dataset, e.g. PCA
    auto initEmbedding = _tsneSettingsAction->getInitalEmbeddingSettingsAction().getInitEmbedding(numPoints);

    _dataPreparationTask.setFinished();

    _tsneAnalysis.startComputation(_tsneSettingsAction->getTsneParameters(), _tsneSettingsAction->getKnnParameters(), std::move(data), numEnabledDimensions, &initEmbedding);
}

void TsneAnalysisPlugin::reinitializeComputation()
{
    if (_tsneAnalysis.canContinue())
        _probDistMatrix = std::move(*_tsneAnalysis.getProbabilityDistribution().value());
    
    if(_probDistMatrix.size() == 0)
    {
        qDebug() << "TsneAnalysisPlugin::reinitializeComputation: cannot reinitialize embedding - start computation first";
        return;
    }

    auto& initSettings = _tsneSettingsAction->getInitalEmbeddingSettingsAction();

    if (initSettings.getRandomInitAction().isChecked() && initSettings.getNewRandomSeedAction().isChecked())
        initSettings.updateSeed();

    const auto numPoints = getOutputDataset<Points>()->getNumPoints();

    auto initEmbedding = initSettings.getInitEmbedding(numPoints);

    _tsneAnalysis.startComputation(_tsneSettingsAction->getTsneParameters(), std::move(_probDistMatrix), numPoints, &initEmbedding);
}

void TsneAnalysisPlugin::continueComputation()
{
    getOutputDataset()->getTask().setRunning();

    _dataPreparationTask.setEnabled(false);

    _tsneSettingsAction->getComputationAction().getRunningAction().setChecked(true);

    if (_tsneAnalysis.canContinue())
        _tsneAnalysis.continueComputation(_tsneSettingsAction->getTsneParameters().getNumIterations());
    else if (_probDistMatrix.size() > 0)
    {
        auto currentEmbedding = getOutputDataset<Points>();

        std::vector<float> currentEmbeddingPositions;
        currentEmbeddingPositions.resize(2ull * currentEmbedding->getNumPoints());
        currentEmbedding->populateDataForDimensions<std::vector<float>, std::vector<unsigned int>>(currentEmbeddingPositions, { 0, 1 });

        _tsneAnalysis.startComputation(_tsneSettingsAction->getTsneParameters(), std::move(_probDistMatrix), currentEmbedding->getNumPoints(), &currentEmbeddingPositions, _tsneSettingsAction->getGeneralTsneSettingsAction().getNumberOfComputatedIterationsAction().getValue());
    }
    else
    {
        qWarning() << "TsneAnalysisPlugin::continueComputation: cannot continue.";
        _tsneSettingsAction->getComputationAction().getRunningAction().setChecked(false);
        _dataPreparationTask.setEnabled(false);
        getOutputDataset()->getTask().setFinished();
    }
}

void TsneAnalysisPlugin::stopComputation()
{
    _tsneAnalysis.stopComputation();
}

void TsneAnalysisPlugin::fromVariantMap(const QVariantMap& variantMap)
{
    AnalysisPlugin::fromVariantMap(variantMap);

    variantMapMustContain(variantMap, "TSNE Settings");
    _tsneSettingsAction->fromVariantMap(variantMap["TSNE Settings"].toMap());

    if (_tsneSettingsAction->getGeneralTsneSettingsAction().getSaveProbDistAction().isChecked())
    {
        if (variantMap.contains("probabilityDistribution"))
        {
            // Load HSNE Hierarchy
            const auto loadPathHierarchy = QDir::cleanPath(projects().getTemporaryDirPath(AbstractProjectManager::TemporaryDirType::Open) + QDir::separator() + variantMap["probabilityDistribution"].toString());

            std::ifstream loadFile(loadPathHierarchy.toStdString().c_str(), std::ios::in | std::ios::binary);

            if (loadFile.is_open())
            {
                hdi::data::IO::loadSparseMatrix(_probDistMatrix, loadFile, nullptr);

                _tsneSettingsAction->getComputationAction().getContinueComputationAction().setEnabled(true);
            }
            else
                qWarning("TsneAnalysisPlugin::fromVariantMap: t-SNE probability distribution was NOT loaded successfully");
        }
        else
            qWarning("TsneAnalysisPlugin::fromVariantMap: t-SNE probability distribution cannot be loaded from project since the project file does not seem to contain a corresponding file.");
    }
}

QVariantMap TsneAnalysisPlugin::toVariantMap() const
{
    QVariantMap variantMap = AnalysisPlugin::toVariantMap();

    _tsneSettingsAction->insertIntoVariantMap(variantMap);

    const auto probabilityDistribution = _tsneAnalysis.getProbabilityDistribution();

    if (_tsneSettingsAction->getGeneralTsneSettingsAction().getSaveProbDistAction().isChecked() && probabilityDistribution != std::nullopt)
    {
        const auto fileName = QUuid::createUuid().toString(QUuid::WithoutBraces) + ".bin";
        const auto filePath = QDir::cleanPath(projects().getTemporaryDirPath(AbstractProjectManager::TemporaryDirType::Save) + QDir::separator() + fileName).toStdString();

        std::ofstream saveFile(filePath, std::ios::out | std::ios::binary);

        if (!saveFile.is_open())
            std::cerr << "Caching failed. File could not be opened. " << std::endl;
        else
        {
            hdi::data::IO::saveSparseMatrix(*probabilityDistribution.value(), saveFile, nullptr);
            saveFile.close();
            variantMap["probabilityDistribution"] = fileName;
        }
    }

    return variantMap;
}

// =============================================================================
// Plugin Factory 
// =============================================================================

QIcon TsneAnalysisPluginFactory::getIcon(const QColor& color /*= Qt::black*/) const
{
    return createPluginIcon("TSNE", color);
}

AnalysisPlugin* TsneAnalysisPluginFactory::produce()
{
    return new TsneAnalysisPlugin(this);
}

mv::DataTypes TsneAnalysisPluginFactory::supportedDataTypes() const
{
    return { PointType };
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
                for (const auto& dataset : datasets)
                    getPluginInstance(dataset);
            });

            pluginTriggerActions << pluginTriggerAction;
        }

        if (datasets.count() >= 2) {
            auto pluginTriggerAction = new PluginTriggerAction(const_cast<TsneAnalysisPluginFactory*>(this), this, "Group/TSNE", "Group datasets and perform TSNE analysis on it", getIcon(), [this, getPluginInstance, datasets](PluginTriggerAction& pluginTriggerAction) -> void {
                getPluginInstance(mv::data().groupDatasets(datasets));
            });

            pluginTriggerActions << pluginTriggerAction;
        }
    }

    return pluginTriggerActions;
}

