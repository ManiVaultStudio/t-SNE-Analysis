#include "HsneAnalysisPlugin.h"
#include "HsneParameters.h"
#include "HsneScaleAction.h"

#include "PointData/InfoAction.h"
#include <PointData/PointData.h>

#include <actions/PluginTriggerAction.h>
#include <util/Icon.h>

#include <QDebug>
#include <QByteArray>
#include <QDataStream>

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
    // Created derived dataset for embedding
    setOutputDataset(mv::data().createDerivedDataset("HSNE Embedding", getInputDataset(), getInputDataset()));

    // Create new HSNE settings actions
    _hsneSettingsAction = new HsneSettingsAction(this);

    // Collapse the TSNE settings by default
    _hsneSettingsAction->getTsneSettingsAction().getGeneralTsneSettingsAction().collapse();

    // Get input/output datasets
    auto inputDataset  = getInputDataset<Points>();
    auto outputDataset = getOutputDataset<Points>();

    outputDataset->getDataHierarchyItem().select();
    outputDataset->_infoAction->collapse();

    _dataPreparationTask.setParentTask(&outputDataset->getTask());

    // Set the default number of hierarchy scales based on number of points
    int numHierarchyScales = std::max(1L, std::lround(log10(inputDataset->getNumPoints())) - 2);
    _hsneSettingsAction->getGeneralHsneSettingsAction().getNumScalesAction().setValue(numHierarchyScales);

    std::vector<float> initialData;

    const size_t numEmbeddingDimensions = 2;

    initialData.resize(numEmbeddingDimensions * inputDataset->getNumPoints());

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
        
        qApp->processEvents();

        std::vector<bool> enabledDimensions = getInputDataset<Points>()->getDimensionsPickerAction().getEnabledDimensions();

        // Initialize the HSNE algorithm with the given parameters and compute the hierarchy
        _hierarchy.initialize(*getInputDataset<Points>(), enabledDimensions, _hsneSettingsAction->getHsneParameters());

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

    datasetTask.setName("Compute HSNE");
    datasetTask.setConfigurationFlag(Task::ConfigurationFlag::OverrideAggregateStatus);

    _tsneAnalysis.setTask(&datasetTask);
}

void HsneAnalysisPlugin::computeTopLevelEmbedding()
{
    getOutputDataset()->getTask().setRunning();

    _dataPreparationTask.setRunning();

    // Get the top scale of the HSNE hierarchy
    int topScaleIndex = _hierarchy.getTopScale();
    Hsne::scale_type& topScale = _hierarchy.getScale(topScaleIndex);
    
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
    mv::Dataset<Points> subset = inputDataset->createSubsetFromSelection(QString("hsne_scale_%1").arg(topScaleIndex), nullptr, false);

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
            std::vector<unsigned int> globalIndices;
            subset->getGlobalIndices(globalIndices);

            for (int i = 0; i < landmarkMap.size(); i++)
            {
                int bottomLevelIdx = _hierarchy.getScale(topScaleIndex)._landmark_to_original_data_idx[i];
                
                mapping.getMap()[globalIndices[i]] = landmarkMap[i];
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

    _dataPreparationTask.setFinished();

    // Embed data
    _tsneAnalysis.stopComputation();
    _tsneAnalysis.startComputation(tsneParameters, _hierarchy.getTransitionMatrixAtScale(topScaleIndex), numLandmarks, _hierarchy.getNumDimensions());
}

void HsneAnalysisPlugin::continueComputation()
{
    getOutputDataset()->getTask().setRunning();

    _dataPreparationTask.setEnabled(false);

    _hsneSettingsAction->getTsneSettingsAction().getComputationAction().getRunningAction().setChecked(true);

    _tsneAnalysis.continueComputation(_hsneSettingsAction->getTsneSettingsAction().getTsneParameters().getNumIterations());
}

typedef std::vector<unsigned int> InnerVector;
typedef std::vector<InnerVector> MiddleVector;
typedef std::vector<MiddleVector> OuterVector;

static QByteArray serializeNestedVector(const OuterVector& inputVector)
{
    QByteArray byteArray;
    QDataStream stream(&byteArray, QIODevice::WriteOnly);

    // Serialize the vector
    stream << static_cast<quint32>(inputVector.size()); // Store the size of the outer vector

    for (const auto& middleVector : inputVector) {
        stream << static_cast<quint32>(middleVector.size()); // Store the size of the middle vector

        for (const auto& innerVector : middleVector) {
            stream << static_cast<quint32>(innerVector.size()); // Store the size of the inner vector

            for (const auto& value : innerVector) {
                stream << value;
            }
        }
    }

    return byteArray;
}

static OuterVector deserializeNestedVector(const QByteArray& byteArray)
{
    OuterVector result;
    QDataStream stream(byteArray);

    // Deserialize the vector
    quint32 outerSize;
    stream >> outerSize;

    for (quint32 i = 0; i < outerSize; ++i) {
        MiddleVector middleVector;

        quint32 middleSize;
        stream >> middleSize;

        for (quint32 j = 0; j < middleSize; ++j) {
            InnerVector innerVector;

            quint32 valueSize;
            stream >> valueSize;

            for (quint32 k = 0; k < valueSize; ++k) {
                unsigned int value;
                stream >> value;
                innerVector.push_back(value);
            }

            middleVector.push_back(innerVector);
        }

        result.push_back(middleVector);
    }

    return result;
}

typedef std::pair<uint, float> PairUIntFloat;
typedef std::vector<PairUIntFloat> PairVector;

static QByteArray serializePairVector(const PairVector& inputVector)
{
    QByteArray byteArray;
    QDataStream stream(&byteArray, QIODevice::WriteOnly);

    // Serialize the vector
    stream << static_cast<quint32>(inputVector.size()); // Store the size of the vector

    for (const auto& pair : inputVector) {
        stream << pair.first;
        stream << pair.second;
    }

    return byteArray;
}

static PairVector deserializePairVector(const QByteArray& byteArray)
{
    PairVector result;
    QDataStream stream(byteArray);

    // Deserialize the vector
    quint32 size;
    stream >> size;

    for (quint32 i = 0; i < size; ++i) {
        PairUIntFloat pair;
        stream >> pair.first;
        stream >> pair.second;
        result.push_back(pair);
    }

    return result;
}

void HsneAnalysisPlugin::fromVariantMap(const QVariantMap& variantMap)
{
    AnalysisPlugin::fromVariantMap(variantMap);

    variantMapMustContain(variantMap, "hsneSettings");

    _hsneSettingsAction->fromVariantMap(variantMap["hsneSettings"].toMap());

    // Handle HSNE Hierarchy
    std::vector<bool> enabledDimensions = getInputDataset<Points>()->getDimensionsPickerAction().getEnabledDimensions();
    _hierarchy.setDataAndParameters(*getInputDataset<Points>(), enabledDimensions, _hsneSettingsAction->getHsneParameters());

    auto& hsne = _hierarchy.getHsne();

    unsigned int numEnabledDimensions = count_if(enabledDimensions.begin(), enabledDimensions.end(), [](bool b) { return b; });
    hsne.setDimensionality(numEnabledDimensions);

    hsne.hierarchy().clear();
    int numScales = variantMap["numberScales"].toInt();

    for (int numScale = 0; numScale < numScales; numScale++)
    {
        hsne.hierarchy().push_back(typename Hsne::Scale());
        auto& scale = hsne.scale(numScale);

        auto scaleName = "Raw Data " + std::to_string(numScale);
        const auto scaleMap = variantMap[scaleName.c_str()].toInt();

        auto numRowsTransEntry = "Num Rows Trans" + std::to_string(numScale);
        size_t numRowsTrans = static_cast<size_t>(variantMap[numRowsTransEntry.c_str()].toInt());
        
        // transition matrix
        scale._transition_matrix.resize(numRowsTrans);

        for (size_t row = 0; row < numRowsTrans; row++)
        {
            if (row % 1000 == 0)
                qDebug() << "HsneAnalysisPlugin::fromVariantMap: transition matrix " << row << " of " << numRowsTrans;

            auto columnSizeEntry = "Transition " + std::to_string(numScale) + " Row " + std::to_string(row) + " Column Size";
            auto dataEntry = "Transition " + std::to_string(numScale) + " Row " + std::to_string(row) + " Data";

            size_t numCols = static_cast<size_t>(variantMap[columnSizeEntry.c_str()].toInt());
            const auto colData = variantMap[dataEntry.c_str()].toMap();

            std::vector<std::pair<uint32_t, float>> dataVec;
            populateDataBufferFromVariantMap(colData, (char*)dataVec.data());

            scale._transition_matrix[row].memory() = std::move(dataVec);
        }

        // landmarks to original data 
        {
            auto landmarkToOrigEntry = "landmarkToOrig " + std::to_string(numScale);
            const auto data = variantMap[landmarkToOrigEntry.c_str()].toMap();
            std::vector<uint32_t> dataVec;
            populateDataBufferFromVariantMap(data, (char*)dataVec.data());
            scale._landmark_to_original_data_idx = std::move(dataVec);
        }

        // landmarks to previous scale 
        {
            auto landmarkToPrevEntry = "landmarkToPrev " + std::to_string(numScale);
            const auto data = variantMap[landmarkToPrevEntry.c_str()].toMap();
            std::vector<uint32_t> dataVec;
            populateDataBufferFromVariantMap(data, (char*)dataVec.data());
            scale._landmark_to_previous_scale_idx = std::move(dataVec);
        }

        // landmark weights 
        {
            auto landmarkWeightEntry = "landmarkWeight " + std::to_string(numScale);
            const auto data = variantMap[landmarkWeightEntry.c_str()].toMap();
            std::vector<float> dataVec;
            populateDataBufferFromVariantMap(data, (char*)dataVec.data());
            scale._landmark_weight = std::move(dataVec);
        }

        // previous scale to current scale landmarks 
        {
            auto previousToIdxEntry = "previousToIdx " + std::to_string(numScale);
            const auto data = variantMap[previousToIdxEntry.c_str()].toMap();
            std::vector<int32_t> dataVec;
            populateDataBufferFromVariantMap(data, (char*)dataVec.data());
            scale._previous_scale_to_landmark_idx = std::move(dataVec);
        }

        // area of influence 

        auto numRowsAoIEntry = "Num Rows AoI " + std::to_string(numScale);
        size_t numRowsAoI = static_cast<size_t>(variantMap[numRowsAoIEntry.c_str()].toInt());

        scale._area_of_influence.resize(numRowsAoI);

        for (size_t row = 0; row < numRowsAoI; row++)
        {
            if (row % 1000 == 0)
                qDebug() << "HsneAnalysisPlugin::fromVariantMap: area of influence " << row << " of " << numRowsAoI;

            auto columnSizeEntry = "AoI " + std::to_string(numScales) + " Row " + std::to_string(row) + "Column Size";
            auto dataEntry = "AoI " + std::to_string(numScales) + " Row " + std::to_string(row) + " Data";

            size_t numCols = static_cast<size_t>(variantMap[columnSizeEntry.c_str()].toInt());
            const auto colData = variantMap[dataEntry.c_str()].toMap();

            std::vector<std::pair<uint32_t, float>> dataVec;
            populateDataBufferFromVariantMap(colData, (char*)dataVec.data());

            scale._area_of_influence[row].memory() = std::move(dataVec);
        }

    }
}

static QList<std::string> saved;

QVariantMap HsneAnalysisPlugin::toVariantMap() const
{
    if (saved.contains(this->getId().toStdString()))
        return {};
    else
        saved.push_back(this->getId().toStdString());

    qDebug() << "HsneAnalysisPlugin::toVariantMap: " << saved;

    QVariantMap variantMap = AnalysisPlugin::toVariantMap();

    _hsneSettingsAction->insertIntoVariantMap(variantMap);

    // Handle HSNE Hierarchy
    auto numScales = _hierarchy.getNumScales();
    variantMap["numberScales"] = numScales;

    for(int numScale = 0; numScale < numScales; numScale++)
    {
        qDebug() << "HsneAnalysisPlugin::toVariantMap: scale " << numScale;

        const auto& scale = _hierarchy.getScale(numScale);
        QVariantMap scaleData;

        // transition matrix
        qDebug() << "HsneAnalysisPlugin::toVariantMap: transition matrix";
        {
            auto numRowsTrans = scale._transition_matrix.size();
            auto numRowsTransEntry = "Num Rows Trans" + std::to_string(numScale);
            scaleData[numRowsTransEntry.c_str()] = numRowsTrans;

            for (size_t row = 0; row < numRowsTrans; row++)
            {
                if (row % 1000 == 0)
                    qDebug() << "HsneAnalysisPlugin::toVariantMap: transition matrix " << row << " of " << numRowsTrans;

                auto dataEntry = "Transition " + std::to_string(numScale) + " Row " + std::to_string(row) + " Data";
                scaleData[dataEntry.c_str()] = serializePairVector(scale._transition_matrix[row].memory());

            }

        }

        // landmarks to original data 
        {
            const std::vector<uint32_t>& landmarkToOrig = scale._landmark_to_original_data_idx;
            auto landmarkToOrigEntry = "landmarkToOrig " + std::to_string(numScale);
            scaleData[landmarkToOrigEntry.c_str()] = rawDataToVariantMap((char*)landmarkToOrig.data(), landmarkToOrig.size() * sizeof(uint32_t), true);
        }

        // landmarks to previous scale 
        {
            const std::vector<uint32_t>& landmarkToPrev = scale._landmark_to_previous_scale_idx;
            auto landmarkToPrevEntry = "landmarkToPrev " + std::to_string(numScale);
            scaleData[landmarkToPrevEntry.c_str()] = rawDataToVariantMap((char*)landmarkToPrev.data(), landmarkToPrev.size() * sizeof(uint32_t), true);
        }

        // landmark weights 
        {
            const std::vector<float>& landmarkWeight = scale._landmark_weight;
            auto landmarkWeightEntry = "landmarkWeight " + std::to_string(numScale);
            scaleData[landmarkWeightEntry.c_str()] = rawDataToVariantMap((char*)landmarkWeight.data(), landmarkWeight.size() * sizeof(float), true);
        }

        // previous scale to current scale landmarks 
        {
            const std::vector<int32_t>& previousToIdx = scale._previous_scale_to_landmark_idx;
            auto previousToIdxEntry = "previousToIdx " + std::to_string(numScale);
            scaleData[previousToIdxEntry.c_str()] = rawDataToVariantMap((char*)previousToIdx.data(), previousToIdx.size() * sizeof(int32_t), true);
        }

        // area of influence 
        {
            qDebug() << "HsneAnalysisPlugin::toVariantMap: area of influence";
            auto numRowsAoI = scale._area_of_influence.size();

            auto numRowsAoIEntry = "Num Rows AoI" + std::to_string(numScale);
            scaleData[numRowsAoIEntry.c_str()] = numRowsAoI;

            for (size_t row = 0; row < numRowsAoI; row++)
            {
                if (row % 1000 == 0)
                    qDebug() << "HsneAnalysisPlugin::toVariantMap: area of influence " << row << " of " << numRowsAoI;


                auto dataEntry = "AoI " + std::to_string(numScale) + " Row " + std::to_string(row) + " Data";
                scaleData[dataEntry.c_str()] = serializePairVector(scale._area_of_influence[row].memory());
            }
        }

        auto mapName = "Raw Data " + std::to_string(numScale);
        variantMap.insert({ { mapName.c_str(), QVariant::fromValue(scaleData)} });
    }

    // Influence Hierarchy
    qDebug() << "HsneAnalysisPlugin::toVariantMap: Influence Hierarchy";
    {
        //QVariantMap influenceHierarchyMap;
        const std::vector < std::vector<std::vector<unsigned int>>>& influenceHierarchy = _hierarchy.getInfluenceHierarchy().getMap();

        variantMap.insert({ {"influenceHierarchySize", QVariant::fromValue(influenceHierarchy.size())} });

        variantMap["InfluenceHierarchy"] = serializeNestedVector(influenceHierarchy);
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