#include "HsneAnalysisPlugin.h"
#include "HsneParameters.h"
#include "HsneScaleAction.h"

#include <PointData/PointData.h>

#include <util/Icon.h>
#include <actions/PluginTriggerAction.h>

#include <QDebug>
#include <QPainter>

Q_PLUGIN_METADATA(IID "nl.tudelft.HsneAnalysisPlugin")

#ifdef _WIN32
    #include <windows.h>
#endif

using namespace mv;
using namespace mv::util;

HsneAnalysisPlugin::HsneAnalysisPlugin(const PluginFactory* factory) :
    AnalysisPlugin(factory),
    _hierarchy(),
    _tsneAnalysis(),
    _hsneSettingsAction(nullptr),
    _initializationTask(this, "Initializing HSNE")
{
    setObjectName("HSNE");

    _initializationTask.setDescription("All operations prior to HSNE computation");
}

HsneAnalysisPlugin::~HsneAnalysisPlugin()
{
}

void HsneAnalysisPlugin::init()
{
    HsneScaleAction::core = _core;

    // Created derived dataset for embedding
    setOutputDataset(_core->createDerivedDataset("HSNE Embedding", getInputDataset(), getInputDataset()));

    getOutputDataset()->getDataHierarchyItem().select();

    // Create new HSNE settings actions
    _hsneSettingsAction = new HsneSettingsAction(this);

    // Collapse the TSNE settings by default
    _hsneSettingsAction->getTsneSettingsAction().getGeneralTsneSettingsAction().collapse();

    // Get input/output datasets
    auto inputDataset  = getInputDataset<Points>();
    auto outputDataset = getOutputDataset<Points>();

    _initializationTask.setParentTask(&outputDataset->getTask());

    // Set the default number of hierarchy scales based on number of points
    int numHierarchyScales = std::max(1L, std::lround(log10(inputDataset->getNumPoints())) - 2);
    _hsneSettingsAction->getGeneralHsneSettingsAction().getNumScalesAction().setValue(numHierarchyScales);

    std::vector<float> initialData;

    const auto numEmbeddingDimensions = 2;

    initialData.resize(inputDataset->getNumPoints() * numEmbeddingDimensions);

    outputDataset->setData(initialData.data(), inputDataset->getNumPoints(), numEmbeddingDimensions);

    outputDataset->addAction(_hsneSettingsAction->getGeneralHsneSettingsAction());
    outputDataset->addAction(_hsneSettingsAction->getAdvancedHsneSettingsAction());
    outputDataset->addAction(_hsneSettingsAction->getTopLevelScaleAction());
    outputDataset->addAction(_hsneSettingsAction->getTsneSettingsAction().getGeneralTsneSettingsAction());
    outputDataset->addAction(_hsneSettingsAction->getTsneSettingsAction().getAdvancedTsneSettingsAction());
    
    auto dimensionsGroupAction = new GroupAction(this, "Dimensions", true);

    dimensionsGroupAction->addAction(&inputDataset->getFullDataset<Points>()->getDimensionsPickerAction());

    dimensionsGroupAction->setText(QString("Input dimensions (%1)").arg(inputDataset->getFullDataset<Points>()->text()));
    dimensionsGroupAction->setShowLabels(false);

    outputDataset->addAction(*dimensionsGroupAction);

    outputDataset->getDataHierarchyItem().select();

    auto& computationAction = _hsneSettingsAction->getTsneSettingsAction().getComputationAction();

    const auto updateComputationAction = [this, &computationAction]() {
        const auto isRunning = computationAction.getRunningAction().isChecked();

        computationAction.getStartComputationAction().setEnabled(!isRunning);
        computationAction.getContinueComputationAction().setEnabled(!isRunning && _tsneAnalysis.canContinue());
        computationAction.getStopComputationAction().setEnabled(isRunning);
    };

    connect(&_tsneAnalysis, &TsneAnalysis::finished, this, [this, &computationAction, updateComputationAction]() {
        computationAction.getRunningAction().setChecked(false);

        _hsneSettingsAction->getGeneralHsneSettingsAction().getStartAction().setEnabled(false);
        _hsneSettingsAction->setReadOnly(false);

        updateComputationAction();
    });

    connect(&_tsneAnalysis, &TsneAnalysis::aborted, this, [this, &computationAction, updateComputationAction]() {
        updateComputationAction();

        computationAction.getRunningAction().setChecked(false);

        _hsneSettingsAction->getTsneSettingsAction().getGeneralTsneSettingsAction().setReadOnly(false);
        _hsneSettingsAction->getTsneSettingsAction().getAdvancedTsneSettingsAction().setReadOnly(false);
    });

    connect(&computationAction.getStartComputationAction(), &TriggerAction::triggered, this, [this, &computationAction]() {
        _hsneSettingsAction->getTsneSettingsAction().getGeneralTsneSettingsAction().setReadOnly(true);
        _hsneSettingsAction->getTsneSettingsAction().getAdvancedTsneSettingsAction().setReadOnly(true);

        int topScaleIndex = _hierarchy.getTopScale();
        Hsne::scale_type& topScale = _hierarchy.getScale(topScaleIndex);
        int numLandmarks = topScale.size();
        TsneParameters tsneParameters = _hsneSettingsAction->getTsneSettingsAction().getTsneParameters();

        _tsneAnalysis.startComputation(tsneParameters, _hierarchy.getTransitionMatrixAtScale(topScaleIndex), numLandmarks, _hierarchy.getNumDimensions());
    });

    connect(&computationAction.getContinueComputationAction(), &TriggerAction::triggered, this, [this]() {
        _hsneSettingsAction->getTsneSettingsAction().getGeneralTsneSettingsAction().setReadOnly(true);
        _hsneSettingsAction->getTsneSettingsAction().getAdvancedTsneSettingsAction().setReadOnly(true);

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
        
        _initializationTask.setRunning();

        qApp->processEvents();

        std::vector<bool> enabledDimensions = getInputDataset<Points>()->getDimensionsPickerAction().getEnabledDimensions();

        // Initialize the HSNE algorithm with the given parameters
        _hierarchy.initialize(_core, *getInputDataset<Points>(), enabledDimensions, _hsneSettingsAction->getHsneParameters());

        _initializationTask.setFinished();

        qApp->processEvents();

        computeTopLevelEmbedding();
    });

    connect(&_tsneAnalysis, &TsneAnalysis::embeddingUpdate, this, [this](const TsneData& tsneData) {
        auto embedding = getOutputDataset<Points>();

        embedding->setData(tsneData.getData().data(), tsneData.getNumPoints(), 2);

        _hsneSettingsAction->getTsneSettingsAction().getGeneralTsneSettingsAction().getNumberOfComputatedIterationsAction().setValue(_tsneAnalysis.getNumIterations() - 1);

        // NOTE: Commented out because it causes a stack overflow after a couple of iterations
        //QCoreApplication::processEvents();

        events().notifyDatasetDataChanged(getOutputDataset());
    });

    updateComputationAction();

    auto& datasetTask = getOutputDataset()->getTask();

    datasetTask.setName("TSNE Computation");
    datasetTask.setConfigurationFlag(Task::ConfigurationFlag::OverrideAggregateStatus);

    _tsneAnalysis.setTask(&datasetTask);
}

void HsneAnalysisPlugin::computeTopLevelEmbedding()
{
    getOutputDataset()->getTask().setRunning();

    _initializationTask.setEnabled(true);
    _initializationTask.setRunning();

    // Get the top scale of the HSNE hierarchy
    int topScaleIndex = _hierarchy.getTopScale();

    Hsne::scale_type& topScale = _hierarchy.getScale(topScaleIndex);
    
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
    auto subset = inputDataset->createSubsetFromSelection(QString("hsne_scale_%1").arg(topScaleIndex), nullptr, false);

    selectionDataset->indices.clear();

    auto embeddingDataset = getOutputDataset<Points>();
    
    embeddingDataset->setSourceDataSet(subset);
    _hsneSettingsAction->getTopLevelScaleAction().setScale(topScaleIndex);

    _hierarchy.printScaleInfo();

    // Set t-SNE parameters
    HsneParameters hsneParameters = _hsneSettingsAction->getHsneParameters();
    TsneParameters tsneParameters = _hsneSettingsAction->getTsneSettingsAction().getTsneParameters();

    // Add linked selection between the upper embedding and the bottom layer
    {
        LandmarkMap& landmarkMap = _hierarchy.getInfluenceHierarchy().getMap()[topScaleIndex];
        
        mv::SelectionMap mapping;

        if (inputDataset->isFull())
        {
            for (int i = 0; i < landmarkMap.size(); i++)
            {
                int bottomLevelIdx = _hierarchy.getScale(topScaleIndex)._landmark_to_original_data_idx[i];
                mapping.getMap()[bottomLevelIdx] = landmarkMap[i];
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
                mapping.getMap()[globalIndices[bottomLevelIdx]] = bottomMap;
            }
        }

        embeddingDataset->addLinkedData(inputDataset, mapping);
    }

    _initializationTask.setFinished();

    // Embed data
    _tsneAnalysis.stopComputation();
    _tsneAnalysis.startComputation(tsneParameters, _hierarchy.getTransitionMatrixAtScale(topScaleIndex), numLandmarks, _hierarchy.getNumDimensions());
}

void HsneAnalysisPlugin::continueComputation()
{
    getOutputDataset()->getTask().setRunning();

    _initializationTask.setEnabled(false);

    _hsneSettingsAction->getTsneSettingsAction().getComputationAction().getRunningAction().setChecked(true);

    _tsneAnalysis.continueComputation(_hsneSettingsAction->getTsneSettingsAction().getTsneParameters().getNumIterations());
}

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
                for (auto dataset : datasets)
                    getPluginInstance(dataset);
            });

            pluginTriggerActions << pluginTriggerAction;
        }

        if (datasets.count() >= 2) {
            auto pluginTriggerAction = new PluginTriggerAction(const_cast<HsneAnalysisPluginFactory*>(this), this, "Group/HSNE", "Group datasets and perform HSNE analysis on it", getIcon(), [this, getPluginInstance, datasets](PluginTriggerAction& pluginTriggerAction) -> void {
                getPluginInstance(Application::core()->groupDatasets(datasets));
            });

            pluginTriggerActions << pluginTriggerAction;
        }
    }

    return pluginTriggerActions;
}