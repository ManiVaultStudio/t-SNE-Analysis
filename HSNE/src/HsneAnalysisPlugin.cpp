#include "HsneAnalysisPlugin.h"
#include "HsneParameters.h"
#include "HsneScaleAction.h"

#include <PointData/DimensionsPickerAction.h>
#include <PointData/InfoAction.h>
#include <PointData/PointData.h>

#include <actions/PluginTriggerAction.h>
#include <util/Icon.h>

#include "hdi/dimensionality_reduction/hierarchical_sne.h"

#include <fstream>

Q_PLUGIN_METADATA(IID "nl.tudelft.HsneAnalysisPlugin")

using namespace mv;
using namespace mv::util;

HsneAnalysisPlugin::HsneAnalysisPlugin(const PluginFactory* factory) :
    AnalysisPlugin(factory),
    _hierarchy(),
    _tsneAnalysis(),
    _hsneSettingsAction(nullptr),
    _dataPreparationTask(this, "Prepare data")
{
    setObjectName("HSNE");

    _dataPreparationTask.setDescription("All operations prior to HSNE computation");
}

HsneAnalysisPlugin::~HsneAnalysisPlugin()
{
}

void HsneAnalysisPlugin::init()
{
    // Get input/output datasets
    auto inputDataset = getInputDataset<Points>();

    // Create derived dataset for embedding
    if (!outputDataInit())
    {
        auto newOutput = Dataset<Points>(mv::data().createDataset("Points", "HSNE Embedding", inputDataset));
        setOutputDataset(newOutput);

        const size_t numEmbeddingDimensions = 2;
        std::vector<float> initialData;
        initialData.resize(numEmbeddingDimensions * inputDataset->getNumPoints());

        newOutput->setData(initialData.data(), inputDataset->getNumPoints(), numEmbeddingDimensions);
        events().notifyDatasetDataChanged(newOutput);
    }
    
    auto outputDataset = getOutputDataset<Points>();

    // Create new HSNE settings actions
    _hsneSettingsAction = new HsneSettingsAction(this);

    _dataPreparationTask.setParentTask(&outputDataset->getTask());

    // Set the default number of hierarchy scales based on number of points
    int numHierarchyScales = std::max(1L, std::lround(log10(inputDataset->getNumPoints())) - 2);
    _hsneSettingsAction->getGeneralHsneSettingsAction().getNumScalesAction().setValue(numHierarchyScales);

    // Manage UI elements attached to output data set
    mv::dataHierarchy().clearSelection();
    outputDataset->getDataHierarchyItem().select();
    outputDataset->_infoAction->collapse();

    outputDataset->addAction(_hsneSettingsAction->getGeneralHsneSettingsAction());
    outputDataset->addAction(_hsneSettingsAction->getHierarchyConstructionSettingsAction());
    outputDataset->addAction(_hsneSettingsAction->getTopLevelScaleAction());
    outputDataset->addAction(_hsneSettingsAction->getGradientDescentSettingsAction());
    outputDataset->addAction(_hsneSettingsAction->getKnnSettingsAction());
    
    auto dimensionsGroupAction = new GroupAction(this, "Dimensions", true);

    dimensionsGroupAction->addAction(&inputDataset->getFullDataset<Points>()->getDimensionsPickerAction());

    dimensionsGroupAction->setText(QString("Input dimensions (%1)").arg(inputDataset->getFullDataset<Points>()->text()));
    dimensionsGroupAction->setShowLabels(false);

    outputDataset->addAction(*dimensionsGroupAction);

    outputDataset->getDataHierarchyItem().select();

    inputDataset->setProperty("selectionHelperCount", 0);

    auto& computationAction = _hsneSettingsAction->getTopLevelScaleAction().getComputationAction();

    const auto updateComputationAction = [this, &computationAction]() {
        const auto isRunning = computationAction.getRunningAction().isChecked();

        computationAction.getStartComputationAction().setEnabled(!isRunning);
        computationAction.getContinueComputationAction().setEnabled(!isRunning && _tsneAnalysis.canContinue());
        computationAction.getStopComputationAction().setEnabled(isRunning);
    };

    connect(&_tsneAnalysis, &TsneAnalysis::finished, this, [this, &computationAction, updateComputationAction]() {
        computationAction.getRunningAction().setChecked(false);

        _hsneSettingsAction->setReadOnly(false);

        updateComputationAction();
    });

    connect(&_tsneAnalysis, &TsneAnalysis::aborted, this, [this, &computationAction, updateComputationAction]() {
        computationAction.getRunningAction().setChecked(false);

        _hsneSettingsAction->setReadOnly(false);

        updateComputationAction();
    });

    connect(&computationAction.getStartComputationAction(), &TriggerAction::triggered, this, [this, &computationAction]() {
        _hsneSettingsAction->setReadOnly(true);

        int topScaleIndex = _hierarchy.getTopScale();
        Hsne::scale_type& topScale = _hierarchy.getScale(topScaleIndex);
        int numLandmarks = topScale.size();
        TsneParameters tsneParameters = _hsneSettingsAction->getTsneParameters();

        _tsneAnalysis.startComputation(tsneParameters, _hierarchy.getTransitionMatrixAtScale(topScaleIndex), numLandmarks, _hierarchy.getNumDimensions());
    });

    connect(&computationAction.getContinueComputationAction(), &TriggerAction::triggered, this, [this]() {
        _hsneSettingsAction->getGradientDescentSettingsAction().setReadOnly(true);

        continueComputation();
    });

    connect(&computationAction.getStopComputationAction(), &TriggerAction::triggered, this, [this]() {
        qApp->processEvents();

        _tsneAnalysis.stopComputation();
    });
    
    connect(&computationAction.getRunningAction(), &ToggleAction::toggled, this, [this, &computationAction, updateComputationAction](bool toggled) {
        getInputDataset<Points>()->getDimensionsPickerAction().setEnabled(!toggled);
        updateComputationAction();
    });

    connect(&_hsneSettingsAction->getGeneralHsneSettingsAction().getStartAction(), &TriggerAction::triggered, this, [this](bool toggled) {
        _hsneSettingsAction->setReadOnly(true);
        
        qApp->processEvents();

        std::vector<bool> enabledDimensions = getInputDataset<Points>()->getDimensionsPickerAction().getEnabledDimensions();

        // Initialize the HSNE algorithm with the given parameters and compute the hierarchy
        _hierarchy.initialize(*getInputDataset<Points>(), enabledDimensions, _hsneSettingsAction->getHsneParameters(), _hsneSettingsAction->getKnnParameters());

        qApp->processEvents();

        computeTopLevelEmbedding();
    });

    connect(&_tsneAnalysis, &TsneAnalysis::embeddingUpdate, this, [this](const TsneData& tsneData) {
        auto embedding = getOutputDataset<Points>();

        embedding->setData(tsneData.getData().data(), tsneData.getNumPoints(), 2);

        _hsneSettingsAction->getTopLevelScaleAction().getNumberOfComputatedIterationsAction().setValue(_tsneAnalysis.getNumIterations() - 1);

        // NOTE: Commented out because it causes a stack overflow after a couple of iterations
        //QCoreApplication::processEvents();

        events().notifyDatasetDataChanged(embedding);
    });

    updateComputationAction();

    // Before the hierarchy is initialized, no embedding re-init is possible
    if (!_hierarchy.itInitialized())
        computationAction.getStartComputationAction().setEnabled(false);

    auto& datasetTask = outputDataset->getTask();

    datasetTask.setName("Compute HSNE");
    datasetTask.setConfigurationFlag(Task::ConfigurationFlag::OverrideAggregateStatus);

    _tsneAnalysis.setTask(&datasetTask);
}

void HsneAnalysisPlugin::computeTopLevelEmbedding()
{
    auto embeddingDataset = getOutputDataset<Points>();

    embeddingDataset->getTask().setRunning();

    _dataPreparationTask.setRunning();

    // Get the top scale of the HSNE hierarchy
    int topScaleIndex = _hierarchy.getTopScale();
    Hsne::scale_type& topScale = _hierarchy.getScale(topScaleIndex);
    _hsneSettingsAction->getTopLevelScaleAction().setScale(topScaleIndex);

    _hierarchy.printScaleInfo();

    // Number of landmarks on the top scale
    int numLandmarks = topScale.size();

    // Create a subset of the points corresponding to the top level HSNE landmarks,
    // Then create an empty embedding derived from this subset
    auto inputDataset       = getInputDataset<Points>();
    auto selectionDataset   = inputDataset->getSelection<Points>();

    // Select the appropriate points to create a subset from
    selectionDataset->indices.resize(numLandmarks);

    if (inputDataset->isFull())
    {
        for (int i = 0; i < numLandmarks; i++)
            selectionDataset->indices[i] = topScale._landmark_to_original_data_idx[i];
    }
    else
    {
        std::vector<unsigned int> globalIndices;
        inputDataset->getGlobalIndices(globalIndices);
        for (int i = 0; i < numLandmarks; i++)
            selectionDataset->indices[i] = globalIndices[topScale._landmark_to_original_data_idx[i]];
    }

    // Create the subset and clear the selection
    auto selectionHelperCount = inputDataset->getProperty("selectionHelperCount").toInt();
    inputDataset->setProperty("selectionHelperCount", ++selectionHelperCount);
    mv::Dataset<Points> subset = inputDataset->createSubsetFromSelection(QString("Hsne selection helper %1").arg(selectionHelperCount), inputDataset, /*visible = */ false);

    selectionDataset->indices.clear();
    
    embeddingDataset->setSourceDataSet(subset);

    // Add linked selection between the upper embedding and the bottom layer
    {
        LandmarkMap& landmarkMap = _hierarchy.getInfluenceHierarchy().getMap()[topScaleIndex];
        
        mv::SelectionMap mapping;
        auto& selectionMap = mapping.getMap();

        if (inputDataset->isFull())
        {
            std::vector<unsigned int> globalIndices;
            subset->getGlobalIndices(globalIndices);

            for (int i = 0; i < landmarkMap.size(); i++)
            {                
                selectionMap[globalIndices[i]] = landmarkMap[i];
            }
        }
        else
        {
            std::vector<unsigned int> globalIndices;
            inputDataset->getGlobalIndices(globalIndices);
            for (int i = 0; i < landmarkMap.size(); i++)
            {
                std::vector<unsigned int> bottomMap = landmarkMap[i];
                for (int j = 0; j < bottomMap.size(); j++)
                {
                    bottomMap[j] = globalIndices[bottomMap[j]];
                }
                int bottomLevelIdx = _hierarchy.getScale(topScaleIndex)._landmark_to_original_data_idx[i];
                selectionMap[globalIndices[bottomLevelIdx]] = bottomMap;
            }
        }

        embeddingDataset->addLinkedData(inputDataset, mapping);
    }

    _dataPreparationTask.setFinished();

    // Set t-SNE parameters
    TsneParameters tsneParameters = _hsneSettingsAction->getTsneParameters();

    // Embed data
    _tsneAnalysis.stopComputation();
    _tsneAnalysis.startComputation(tsneParameters, _hierarchy.getTransitionMatrixAtScale(topScaleIndex), numLandmarks, _hierarchy.getNumDimensions());
}

void HsneAnalysisPlugin::continueComputation()
{
    getOutputDataset()->getTask().setRunning();

    _dataPreparationTask.setEnabled(false);

    _hsneSettingsAction->getTopLevelScaleAction().getComputationAction().getRunningAction().setChecked(true);

    _tsneAnalysis.continueComputation(_hsneSettingsAction->getTsneParameters().getNumIterations());
}

void HsneAnalysisPlugin::fromVariantMap(const QVariantMap& variantMap)
{
    AnalysisPlugin::fromVariantMap(variantMap);

    variantMapMustContain(variantMap, "HSNE Settings");
    _hsneSettingsAction->fromVariantMap(variantMap["HSNE Settings"].toMap());

    std::vector<bool> enabledDimensions = getInputDataset<Points>()->getDimensionsPickerAction().getEnabledDimensions();
    _hierarchy.setDataAndParameters(*getInputDataset<Points>(), enabledDimensions, _hsneSettingsAction->getHsneParameters(), _hsneSettingsAction->getKnnParameters());

    auto& hsne = _hierarchy.getHsne();
    unsigned int numEnabledDimensions = count_if(enabledDimensions.begin(), enabledDimensions.end(), [](bool b) { return b; });
    hsne.setDimensionality(numEnabledDimensions);

    if (_hsneSettingsAction->getHierarchyConstructionSettingsAction().getSaveHierarchyToProjectAction().isChecked())
    {
        if (variantMap.contains("HsneHierarchy") && variantMap.contains("HsneInfluenceHierarchy"))
        {
            hdi::utils::CoutLog log;

            // Load HSNE Hierarchy
            const auto loadPathHierarchy = QDir::cleanPath(projects().getTemporaryDirPath(AbstractProjectManager::TemporaryDirType::Open) + QDir::separator() + variantMap["HsneHierarchy"].toString());
            bool loadedHierarchy = _hierarchy.loadCacheHsneHierarchy(loadPathHierarchy.toStdString(), log);

            // Load HSNE InfluenceHierarchy
            const auto loadPathInfluenceHierarchy = QDir::cleanPath(projects().getTemporaryDirPath(AbstractProjectManager::TemporaryDirType::Open) + QDir::separator() + variantMap["HsneHierarchy"].toString());
            bool loadedInfluenceHierarchy = _hierarchy.loadCacheHsneInfluenceHierarchy(loadPathInfluenceHierarchy.toStdString(), _hierarchy.getInfluenceHierarchy().getMap());
        }
        else
            qWarning("HsneAnalysisPlugin::fromVariantMap: HSNE hierarchy cannot be loaded from project since the project file does not seem to contain a saved HSNE hierarchy");
    }
}

QVariantMap HsneAnalysisPlugin::toVariantMap() const
{
    QVariantMap variantMap = AnalysisPlugin::toVariantMap();

    _hsneSettingsAction->insertIntoVariantMap(variantMap);

    if (_hsneSettingsAction->getHierarchyConstructionSettingsAction().getSaveHierarchyToProjectAction().isChecked())
    {
        // Handle HSNE Hierarchy
        {
            const auto fileName = QUuid::createUuid().toString(QUuid::WithoutBraces) + ".bin";
            const auto filePath = QDir::cleanPath(projects().getTemporaryDirPath(AbstractProjectManager::TemporaryDirType::Save) + QDir::separator() + fileName).toStdString();

            std::ofstream saveFile(filePath, std::ios::out | std::ios::binary);

            if (!saveFile.is_open())
                std::cerr << "Caching failed. File could not be opened. " << std::endl;
            else
            {
                hdi::dr::IO::saveHSNE(_hierarchy.getHsne(), saveFile, nullptr);
                saveFile.close();
                variantMap["HsneHierarchy"] = fileName;
            }
        }

        // Handle HSNE InfluenceHierarchy
        {
            const auto fileName = QUuid::createUuid().toString(QUuid::WithoutBraces) + ".bin";
            const auto filePath = QDir::cleanPath(projects().getTemporaryDirPath(AbstractProjectManager::TemporaryDirType::Save) + QDir::separator() + fileName).toStdString();

            _hierarchy.saveCacheHsneInfluenceHierarchy(filePath, _hierarchy.getInfluenceHierarchy().getMap());
            variantMap["HsneInfluenceHierarchy"] = fileName;
        }
    }

    return variantMap;
}

// =============================================================================
// Plugin Factory 
// =============================================================================

QIcon HsneAnalysisPluginFactory::getIcon(const QColor& color /*= Qt::black*/) const
{
    return createPluginIcon("HSNE", color);
}

AnalysisPlugin* HsneAnalysisPluginFactory::produce()
{
    return new HsneAnalysisPlugin(this);
}

PluginTriggerActions HsneAnalysisPluginFactory::getPluginTriggerActions(const mv::Datasets& datasets) const
{
    PluginTriggerActions pluginTriggerActions;

    const auto getPluginInstance = [this](const Dataset<Points>& dataset) -> HsneAnalysisPlugin* {
        return dynamic_cast<HsneAnalysisPlugin*>(plugins().requestPlugin(getKind(), { dataset }));
    };

    if (PluginFactory::areAllDatasetsOfTheSameType(datasets, PointType)) {
        if (datasets.count() >= 1) {
            auto pluginTriggerAction = new PluginTriggerAction(const_cast<HsneAnalysisPluginFactory*>(this), this, "HSNE", "Perform HSNE analysis on selected datasets", getIcon(), [this, getPluginInstance, datasets](PluginTriggerAction& pluginTriggerAction) -> void {
                for (auto& dataset : datasets)
                    getPluginInstance(dataset);
            });

            pluginTriggerActions << pluginTriggerAction;
        }

        if (datasets.count() >= 2) {
            auto pluginTriggerAction = new PluginTriggerAction(const_cast<HsneAnalysisPluginFactory*>(this), this, "Group/HSNE", "Group datasets and perform HSNE analysis on it", getIcon(), [this, getPluginInstance, datasets](PluginTriggerAction& pluginTriggerAction) -> void {
                getPluginInstance(mv::data().groupDatasets(datasets));
            });

            pluginTriggerActions << pluginTriggerAction;
        }
    }

    return pluginTriggerActions;
}