#include "HsneAnalysisPlugin.h"

#include "HsneParameters.h"
#include "HsneRecomputeWarningDialog.h"
#include "HsneScaleAction.h"

#include <PointData/DimensionsPickerAction.h>
#include <PointData/InfoAction.h>
#include <PointData/PointData.h>

#include <Task.h>

#include <actions/PluginTriggerAction.h>
#include <event/Event.h>
#include <util/Icon.h>

#include "hdi/dimensionality_reduction/hierarchical_sne.h"

#include <fstream>

Q_PLUGIN_METADATA(IID "nl.tudelft.HsneAnalysisPlugin")

using namespace mv;
using namespace mv::util;

HsneAnalysisPlugin::HsneAnalysisPlugin(const PluginFactory* factory) :
    AnalysisPlugin(factory),
    _hierarchy(std::make_unique<HsneHierarchy>()),
    _hierarchyThread(),
    _tsneAnalysis(),
    _hsneSettingsAction(nullptr),
    _selectionHelperData(nullptr)
{
    setObjectName("HSNE");
}

HsneAnalysisPlugin::~HsneAnalysisPlugin()
{
    _hierarchyThread.quit();           // Signal the thread to quit gracefully
    if (!_hierarchyThread.wait(500))   // Wait for the thread to actually finish
        _hierarchyThread.terminate();  // Terminate thread after 0.5 seconds
}

void HsneAnalysisPlugin::init()
{
    // Get input/output datasets
    auto inputDataset = getInputDataset<Points>();

    // Create derived dataset for embedding
    if (!outputDataInit())
    {
        auto newOutput = Dataset<Points>(mv::data().createDerivedDataset("HSNE Embedding", inputDataset, inputDataset));
        setOutputDataset(newOutput);

        const size_t numEmbeddingDimensions = 2;
        newOutput->setData(nullptr, inputDataset->getNumPoints(), numEmbeddingDimensions);

        events().notifyDatasetDataChanged(newOutput);
    }
    
    auto outputDataset = getOutputDataset<Points>();

    // Create new HSNE settings actions
    _hsneSettingsAction = new HsneSettingsAction(this);

    // Set the default number of hierarchy scales based on number of points
    int numHierarchyScales = std::max(1L, std::lround(log10(inputDataset->getNumPoints())) - 2);
    _hsneSettingsAction->getGeneralHsneSettingsAction().getNumScalesAction().setValue(numHierarchyScales);

    // Manage UI elements attached to output data set
    outputDataset->getDataHierarchyItem().select(true);
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

        int topScaleIndex = _hierarchy->getTopScale();
        Hsne::scale_type& topScale = _hierarchy->getScale(topScaleIndex);
        int numLandmarks = topScale.size();
        TsneParameters tsneParameters = _hsneSettingsAction->getTsneParameters();

        _tsneAnalysis.startComputation(tsneParameters, _hierarchy->getTransitionMatrixAtScale(topScaleIndex), numLandmarks);
    });

    connect(&computationAction.getContinueComputationAction(), &TriggerAction::triggered, this, [this]() {
        _hsneSettingsAction->getGradientDescentSettingsAction().setReadOnly(true);

        continueComputation();
    });

    connect(&computationAction.getStopComputationAction(), &TriggerAction::triggered, this, [this]() {
        qApp->processEvents();

        _tsneAnalysis.stopComputation();
    });
    
    connect(&computationAction.getRunningAction(), &ToggleAction::toggled, this, [this, updateComputationAction](bool toggled) {
        getInputDataset<Points>()->getDimensionsPickerAction().setEnabled(!toggled);
        updateComputationAction();
    });

    // once HsneHierarchy::initialize is done, it'll emit HsneHierarchy::finished
    connect(this, &HsneAnalysisPlugin::startHierarchyWorker, _hierarchy.get(), &HsneHierarchy::initialize);

    connect(_hierarchy.get(), &HsneHierarchy::finished, this, [this]() {
        
        _hsneSettingsAction->getGeneralHsneSettingsAction().setReadOnly(false);
        _hsneSettingsAction->getHierarchyConstructionSettingsAction().setReadOnly(false);
        _hsneSettingsAction->getTopLevelScaleAction().setReadOnly(false);
        _hsneSettingsAction->getGradientDescentSettingsAction().setReadOnly(false);
        _hsneSettingsAction->getKnnSettingsAction().setReadOnly(false);

        _hsneSettingsAction->getGeneralHsneSettingsAction().getStartAction().setText("Recompute");
        _hsneSettingsAction->getGeneralHsneSettingsAction().getStartAction().setToolTip("Recomputing does not change the selection mapping.\n If the data size changed, prefer creating a new HSNE analysis.");

        computeTopLevelEmbedding();
    });

    connect(&_hsneSettingsAction->getGeneralHsneSettingsAction().getStartAction(), &TriggerAction::triggered, this, [this](bool toggled) {

        // Create a warning dialog if there are already refined scales
        if (_selectionHelperData.isValid() && getOutputDataset<Points>()->getDataHierarchyItem().getChildren().size() > 0)
        {
            HsneRecomputeWarningDialog dialog;

            if (dialog.exec() == QDialog::Rejected)
                return;
        }

        _tsneAnalysis.stopComputation();

        _hsneSettingsAction->getGeneralHsneSettingsAction().setReadOnly(true);
        _hsneSettingsAction->getHierarchyConstructionSettingsAction().setReadOnly(true);
        _hsneSettingsAction->getTopLevelScaleAction().setReadOnly(true);
        _hsneSettingsAction->getGradientDescentSettingsAction().setReadOnly(true);
        _hsneSettingsAction->getTopLevelScaleAction().getComputationAction().getStartComputationAction().setEnabled(false);
        _hsneSettingsAction->getKnnSettingsAction().setReadOnly(true);

        // Initialize the HSNE algorithm with the given parameters and compute the hierarchy
        auto inputData      = getInputDataset<Points>();
        std::vector<bool> enabledDimensions = inputData->getDimensionsPickerAction().getEnabledDimensions();
        _hierarchy->setDataAndParameters(inputData, getOutputDataset<Points>(), _hsneSettingsAction->getHsneParameters(), _hsneSettingsAction->getKnnParameters(), std::move(enabledDimensions));
        _hierarchy->initParentTask();

        _hierarchy->moveToThread(&_hierarchyThread);

        _hierarchyThread.start();
        emit startHierarchyWorker();

    });

    connect(&_tsneAnalysis, &TsneAnalysis::started, this, [this, &computationAction, updateComputationAction]() {
        computationAction.getRunningAction().setChecked(true);
        updateComputationAction();
        qApp->processEvents();
    });

    connect(&_tsneAnalysis, &TsneAnalysis::embeddingUpdate, this, [this](const TsneData& tsneData) {
        auto embedding = getOutputDataset<Points>();

        embedding->setData(tsneData.getData().data(), tsneData.getNumPoints(), 2);

        _hsneSettingsAction->getTopLevelScaleAction().getNumberOfComputatedIterationsAction().setValue(_tsneAnalysis.getNumIterations() - 1);

        // NOTE: Commented out because it causes a stack overflow after a couple of iterations
        //QCoreApplication::processEvents();

        events().notifyDatasetDataChanged(embedding);
    });

    _eventListener.addSupportedEventType(static_cast<std::uint32_t>(EventType::DatasetAboutToBeRemoved));
    _eventListener.registerDataEventByType(PointType, [this](DatasetEvent* dataEvent) {
        const auto& datasetID       = dataEvent->getDataset()->getId();
        const auto& outputDatasetID = getOutputDataset<Points>()->getId();

        // If the removed dataset is the output data (top level embedding), remove the accompanying _selectionHelperData
        if (outputDatasetID == datasetID)
        {
            if (!_selectionHelperData.isValid())
                return;

            qDebug() << "HSNE Plugin: remove (invisible) selection helper dataset " << _selectionHelperData->getId() << " used for deleted " << datasetID;
            mv::data().removeDataset(_selectionHelperData);
        }

        });

    updateComputationAction();

    // Before the hierarchy is initialized, no embedding re-init is possible
    if (!_hierarchy->isInitialized())
        computationAction.getStartComputationAction().setEnabled(false);

    auto& datasetTask = outputDataset->getTask();

    _tsneAnalysis.setTask(&datasetTask);
}

void HsneAnalysisPlugin::computeTopLevelEmbedding()
{
    auto embeddingDataset = getOutputDataset<Points>();

    auto& datasetTask = embeddingDataset->getTask();
    datasetTask.setFinished();
    datasetTask.setName("Compute HSNE top level embedding");
    datasetTask.setRunning();

    // Get the top scale of the HSNE hierarchy
    int topScaleIndex = _hierarchy->getTopScale();
    Hsne::scale_type& topScale = _hierarchy->getScale(topScaleIndex);
    _hsneSettingsAction->getTopLevelScaleAction().setScale(topScaleIndex);

    _hierarchy->printScaleInfo();

    // Number of landmarks on the top scale
    int numLandmarks = topScale.size();

    // Only create new selection helper if it does not exist yet
    if (!_selectionHelperData.isValid())
    {
        // Create a subset of the points corresponding to the top level HSNE landmarks,
        // Then derive the embedding from this subset
        auto inputDataset = getInputDataset<Points>();
        auto selectionDataset = inputDataset->getSelection<Points>();

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
        _selectionHelperData = inputDataset->createSubsetFromSelection(QString("Hsne selection helper %1").arg(selectionHelperCount), inputDataset, /*visible = */ false);

        selectionDataset->indices.clear();

        embeddingDataset->setSourceDataset(_selectionHelperData);

        // Add linked selection between the upper embedding and the bottom layer
        {
            LandmarkMap& landmarkMap = _hierarchy->getInfluenceHierarchy().getMap()[topScaleIndex];

            mv::SelectionMap mapping;
            auto& selectionMap = mapping.getMap();

            if (inputDataset->isFull())
            {
                std::vector<unsigned int> globalIndices;
                _selectionHelperData->getGlobalIndices(globalIndices);

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
                    int bottomLevelIdx = _hierarchy->getScale(topScaleIndex)._landmark_to_original_data_idx[i];
                    selectionMap[globalIndices[bottomLevelIdx]] = bottomMap;
                }
            }

            embeddingDataset->addLinkedData(inputDataset, mapping);
        }
    }

    // Set t-SNE parameters
    TsneParameters tsneParameters = _hsneSettingsAction->getTsneParameters();

    // Embed data
    _tsneAnalysis.stopComputation();
    _tsneAnalysis.startComputation(tsneParameters, _hierarchy->getTransitionMatrixAtScale(topScaleIndex), numLandmarks);
}

void HsneAnalysisPlugin::continueComputation()
{
    getOutputDataset()->getTask().setRunning();

    _hsneSettingsAction->getTopLevelScaleAction().getComputationAction().getRunningAction().setChecked(true);

    _tsneAnalysis.continueComputation(_hsneSettingsAction->getTsneParameters().getNumIterations());
}

void HsneAnalysisPlugin::fromVariantMap(const QVariantMap& variantMap)
{
    AnalysisPlugin::fromVariantMap(variantMap);

    variantMapMustContain(variantMap, "HSNE Settings");
    variantMapMustContain(variantMap, "selectionHelperDataGUID");

    _hsneSettingsAction->fromVariantMap(variantMap["HSNE Settings"].toMap());

    std::vector<bool> enabledDimensions = getInputDataset<Points>()->getDimensionsPickerAction().getEnabledDimensions();
    _hierarchy->setDataAndParameters(getInputDataset<Points>(), getOutputDataset<Points>(), _hsneSettingsAction->getHsneParameters(), _hsneSettingsAction->getKnnParameters(), std::move(enabledDimensions));

    auto& hsne = _hierarchy->getHsne();
    hsne.setDimensionality(_hierarchy->getNumDimensions());

    if (_hsneSettingsAction->getHierarchyConstructionSettingsAction().getSaveHierarchyToProjectAction().isChecked())
    {
        if (variantMap.contains("HsneHierarchy") && variantMap.contains("HsneInfluenceHierarchy"))
        {
            hdi::utils::CoutLog log;

            // Load HSNE Hierarchy
            const auto loadPathHierarchy = QDir::cleanPath(projects().getTemporaryDirPath(AbstractProjectManager::TemporaryDirType::Open) + QDir::separator() + variantMap["HsneHierarchy"].toString());
            bool loadedHierarchy = _hierarchy->loadCacheHsneHierarchy(loadPathHierarchy.toStdString(), log);

            // Load HSNE InfluenceHierarchy
            const auto loadPathInfluenceHierarchy = QDir::cleanPath(projects().getTemporaryDirPath(AbstractProjectManager::TemporaryDirType::Open) + QDir::separator() + variantMap["HsneInfluenceHierarchy"].toString());
            bool loadedInfluenceHierarchy = _hierarchy->loadCacheHsneInfluenceHierarchy(loadPathInfluenceHierarchy.toStdString(), _hierarchy->getInfluenceHierarchy().getMap());

            _hierarchy->setIsInitialized(true);

            if(!loadedHierarchy || !loadedInfluenceHierarchy)
                qWarning("HsneAnalysisPlugin::fromVariantMap: HSNE hierarchy was NOT loaded successfully");
        }
        else
            qWarning("HsneAnalysisPlugin::fromVariantMap: HSNE hierarchy cannot be loaded from project since the project file does not seem to contain a saved HSNE hierarchy");
    }

    _selectionHelperData = mv::data().getDataset(variantMap["selectionHelperDataGUID"].toString());
    _hsneSettingsAction->getGeneralHsneSettingsAction().getStartAction().setText("Recompute");
    _hsneSettingsAction->getGeneralHsneSettingsAction().getStartAction().setToolTip("Recomputing does not change the selection mapping.\n If the data size changed, prefer creating a new HSNE analysis.");
}

QVariantMap HsneAnalysisPlugin::toVariantMap() const
{
    QVariantMap variantMap = AnalysisPlugin::toVariantMap();

    _hsneSettingsAction->insertIntoVariantMap(variantMap);

    if (_hsneSettingsAction->getHierarchyConstructionSettingsAction().getSaveHierarchyToProjectAction().isChecked() && _hierarchy->isInitialized())
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
                hdi::dr::IO::saveHSNE(_hierarchy->getHsne(), saveFile, nullptr);
                saveFile.close();
                variantMap["HsneHierarchy"] = fileName;
            }
        }

        // Handle HSNE InfluenceHierarchy
        {
            const auto fileName = QUuid::createUuid().toString(QUuid::WithoutBraces) + ".bin";
            const auto filePath = QDir::cleanPath(projects().getTemporaryDirPath(AbstractProjectManager::TemporaryDirType::Save) + QDir::separator() + fileName).toStdString();

            _hierarchy->saveCacheHsneInfluenceHierarchy(filePath, _hierarchy->getInfluenceHierarchy().getMap());
            variantMap["HsneInfluenceHierarchy"] = fileName;
        }
    }

    variantMap["selectionHelperDataGUID"] = QVariant::fromValue(_selectionHelperData->getId());

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