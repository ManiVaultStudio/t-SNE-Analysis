#include "HsneScaleAction.h"

#include "DataHierarchyItem.h"
#include "HsneHierarchy.h"
#include "TsneParameters.h"

#include <event/Event.h>

#include <limits>

#include <QMenu>

using namespace mv;
using namespace mv::gui;

HsneScaleAction::HsneScaleAction(QObject* parent, TsneParameters& tsneParameters, HsneHierarchy& hsneHierarchy, Dataset<Points> inputDataset, Dataset<Points> embeddingDataset) :
    GroupAction(parent, "HSNE Scale", true),
    _tsneParameters(tsneParameters),
    _tsneAnalysis(),
    _hsneHierarchy(hsneHierarchy),
    _input(inputDataset),
    _embedding(embeddingDataset),
    _refineEmbedding(),
    _refineAction(this, "Refine selection"),
    _numIterationsAction(this, "Number of iterations"),
    _numberOfComputatedIterationsAction(this, "Number of computed iterations", 0, std::numeric_limits<int>::max(), 0),
    _updateIterationsAction(this, "Core update every"),
    _computationAction(this),
    _initializationTask(this, "Preparing HSNE scale"),
    _isTopScale(true),
    _currentScaleLevel(0)
{
    addAction(&_refineAction);
    addAction(&_numIterationsAction);
    addAction(&_numberOfComputatedIterationsAction);
//    addAction(&_updateIterationsAction);
    addAction(&_computationAction);

    _numIterationsAction.setDefaultWidgetFlags(IntegralAction::SpinBox);
    _numberOfComputatedIterationsAction.setDefaultWidgetFlags(IntegralAction::LineEdit);
    _updateIterationsAction.setDefaultWidgetFlags(IntegralAction::SpinBox | IntegralAction::Slider);

    _numIterationsAction.initialize(1, 10000, 1000);
    _updateIterationsAction.initialize(0, 10000, 10);

    _refineAction.setToolTip("Refine the selected landmarks");
    _updateIterationsAction.setToolTip("Update the dataset every x iterations. If set to 0, there will be no intermediate result.");

    _numberOfComputatedIterationsAction.setEnabled(false);
    _computationAction.getStartComputationAction().setEnabled(false);

    connect(&_refineAction, &TriggerAction::triggered, this, [this]() {
        refine();
    });

    const auto updateNumIterations = [this]() -> void {
        _tsneParameters.setNumIterations(_numIterationsAction.getValue());
    };

    const auto updateCoreUpdate = [this]() -> void {
        _tsneParameters.setUpdateCore(_updateIterationsAction.getValue());
    };

    const auto updateReadOnly = [this]() -> void {
        auto selection = _input->getSelection<Points>();
        const auto enabled = !isReadOnly();

        _refineAction.setEnabled(!isReadOnly() && !selection->indices.empty());
        _numIterationsAction.setEnabled(enabled);
        _updateIterationsAction.setEnabled(enabled);
    };

    connect(this, &GroupAction::readOnlyChanged, this, [this, updateReadOnly](const bool& readOnly) {
        updateReadOnly();
    });

    connect(&_numIterationsAction, &IntegralAction::valueChanged, this, [this, updateNumIterations](const std::int32_t& value) {
        updateNumIterations();
    });

    connect(&_updateIterationsAction, &IntegralAction::valueChanged, this, [this, updateCoreUpdate](const std::int32_t& value) {
        updateCoreUpdate();
    });

    _eventListener.addSupportedEventType(static_cast<std::uint32_t>(EventType::DatasetDataSelectionChanged));
    _eventListener.registerDataEventByType(PointType, [this, updateReadOnly](DatasetEvent* dataEvent) {
        if (dataEvent->getDataset() == _embedding && dataEvent->getType() == EventType::DatasetDataSelectionChanged)
            updateReadOnly();
    });

    updateReadOnly();
}

QMenu* HsneScaleAction::getContextMenu(QWidget* parent /*= nullptr*/)
{
    auto menu = new QMenu(text(), parent);

    menu->addAction(&_refineAction);

    return menu;
}

void HsneScaleAction::refine()
{
    _initializationTask.setRunning();

    // Get the selection of points that are to be refined
    auto selection = _embedding->getSelection<Points>();

    // Set the scale of the refined embedding to be one below the current scale
    const auto refinedScaleLevel = _currentScaleLevel - 1;

    // Find proper selection indices
    std::vector<bool> selectedLocalIndices;
    _embedding->selectedLocalIndices(selection->indices, selectedLocalIndices);

    // Transform local indices to scale relative indices
    std::vector<unsigned int> selectedLandmarks; // Selected indices relative to scale
    for (int i = 0; i < selectedLocalIndices.size(); i++)
    {
        if (selectedLocalIndices[i])
        {
            selectedLandmarks.push_back(_isTopScale ? i : _drillIndices[i]);
        }
    }
    
    // Find the points in the previous level corresponding to selected landmarks
    std::map<uint32_t, float> neighbors;
    _hsneHierarchy.getInfluencedLandmarksInPreviousScale(_currentScaleLevel, selectedLandmarks, neighbors);

    // Threshold neighbours with enough influence, these represent the indices of the refined points relative to their HSNE scale
    std::vector<uint32_t> refinedLandmarks; // Scale-relative indices
    refinedLandmarks.clear();
    for (const auto& n : neighbors) {
        if (n.second > 0.5) //QUICKPAPER
        {
            refinedLandmarks.push_back(n.first);
        }
    }
    std::cout << "#selected landmarks: " << selectedLandmarks.size() << std::endl;
    std::cout << "#landmarks at refined scale: " << neighbors.size() << std::endl;
    std::cout << "#thresholded landmarks at refined scale: " << refinedLandmarks.size() << std::endl;
    std::cout << "Refining embedding.." << std::endl;
    
    ////////////////////////////
    // Create refined dataset //
    ////////////////////////////
    
    // Compute the transition matrix for the landmarks above the threshold
    HsneMatrix transitionMatrix;
    _hsneHierarchy.getTransitionMatrixForSelection(_currentScaleLevel, transitionMatrix, refinedLandmarks);

    // Create a new data set for the embedding
    {
        auto selection = _input->getSelection<Points>();

        Hsne::scale_type& refinedScale = _hsneHierarchy.getScale(refinedScaleLevel);

        selection->indices.clear();

        if (_input->isFull())
        {
            for (int i = 0; i < refinedLandmarks.size(); i++)
                selection->indices.push_back(refinedScale._landmark_to_original_data_idx[refinedLandmarks[i]]);
        }
        else
        {
            std::vector<unsigned int> globalIndices;
            _input->getGlobalIndices(globalIndices);
            for (int i = 0; i < refinedLandmarks.size(); i++)
                selection->indices.push_back(globalIndices[refinedScale._landmark_to_original_data_idx[refinedLandmarks[i]]]);
        }

        // Create HSNE scale subset
        auto selectionHelperCount = _input->getProperty("selectionHelperCount").toInt();
        _input->setProperty("selectionHelperCount", ++selectionHelperCount);
        auto hsneScaleSubset = _input->createSubsetFromSelection(QString("Hsne selection helper %1").arg(selectionHelperCount), _input, /* visible = */ false);

        // And the derived data for the embedding
        _refineEmbedding = mv::data().createDerivedDataset<Points>(QString("Hsne scale %1").arg(refinedScaleLevel), hsneScaleSubset, _embedding);

        _refineEmbedding->setData(nullptr, 0, 2);
        events().notifyDatasetDataChanged(_refineEmbedding);

        _refineEmbedding->getDataHierarchyItem().select();

        auto& datasetTask = _refineEmbedding->getTask();
        datasetTask.setName("HSNE scale computation");
        datasetTask.setConfigurationFlag(Task::ConfigurationFlag::OverrideAggregateStatus);

        _tsneAnalysis.setTask(&datasetTask);
    }

    // Only add a new scale action if the drill scale is higher than data level
    if (refinedScaleLevel > 0)
    {
        auto hsneScaleAction = new HsneScaleAction(this, _tsneParameters, _hsneHierarchy, _input, _refineEmbedding);
        hsneScaleAction->setDrillIndices(refinedLandmarks);
        hsneScaleAction->setScale(refinedScaleLevel);

        _refineEmbedding->addAction(*hsneScaleAction);
    }

    ///////////////////////////////////
    // Connect scales by linked data //
    ///////////////////////////////////
    
    // Add linked selection between the refined embedding and the bottom level points
    if (refinedScaleLevel > 0) // Only add a linked selection if it's not the bottom level already
    {
        LandmarkMap& landmarkMap = _hsneHierarchy.getInfluenceHierarchy().getMap()[refinedScaleLevel];

        mv::SelectionMap mapping;
        auto& selectionMap = mapping.getMap();

        if (_input->isFull())
        {
            for (const unsigned int& scaleIndex : refinedLandmarks)
            {
                int bottomLevelIdx = _hsneHierarchy.getScale(refinedScaleLevel)._landmark_to_original_data_idx[scaleIndex];
                selectionMap[bottomLevelIdx] = landmarkMap[scaleIndex];
            }
        }
        else
        {
            // Link drill-in points to bottom level indices when the original input to HSNE was a subset
            std::vector<unsigned int> globalIndices;
            _input->getGlobalIndices(globalIndices);
            for (const unsigned int& scaleIndex : refinedLandmarks)
            {
                std::vector<unsigned int> bottomMap = landmarkMap[scaleIndex];
                // Transform bottom level indices to the global full set indices
                for (int j = 0; j < bottomMap.size(); j++)
                {
                    bottomMap[j] = globalIndices[bottomMap[j]];
                }
                int bottomLevelIdx = _hsneHierarchy.getScale(refinedScaleLevel)._landmark_to_original_data_idx[scaleIndex];
                selectionMap[globalIndices[bottomLevelIdx]] = bottomMap;
            }
        }

        _refineEmbedding->addLinkedData(_input, mapping);
    }

    // Update embedding points when the TSNE analysis produces new data
    connect(&_tsneAnalysis, &TsneAnalysis::embeddingUpdate, this, [this](const TsneData& tsneData) {

        // Update the refine embedding with new data
        _refineEmbedding->setData(tsneData.getData().data(), tsneData.getNumPoints(), 2);

        // Notify others that the embedding points have changed
        events().notifyDatasetDataChanged(_refineEmbedding);
    });

    _initializationTask.setFinished();

    // Start the embedding process
    _tsneAnalysis.startComputation(_tsneParameters, transitionMatrix, refinedLandmarks.size(), _hsneHierarchy.getNumDimensions());
}

void HsneScaleAction::fromVariantMap(const QVariantMap& variantMap)
{
    GroupAction::fromVariantMap(variantMap);

    // Handle data sets
    _input = mv::data().getDataset(variantMap["inputGUID"].toString());
    _embedding = mv::data().getDataset(variantMap["embeddingGUID"].toString());

    QString refineEmbeddingGUID = variantMap["refineEmbeddingGUID"].toString();
    if(refineEmbeddingGUID != "")
        _refineEmbedding = mv::data().getDataset(refineEmbeddingGUID);

    {
        const auto drillIndices = variantMap["drillIndices"].toMap();
        std::vector<uint32_t> drillIndicesVec;
        populateDataBufferFromVariantMap(drillIndices, (char*)drillIndicesVec.data());
        _drillIndices = std::move(drillIndicesVec);
    }

    _isTopScale = variantMap["isTopScale"].toBool();
    _currentScaleLevel = variantMap["currentScaleLevel"].toUInt();

    _refineAction.fromParentVariantMap(variantMap);
    _numIterationsAction.fromParentVariantMap(variantMap);
    _numberOfComputatedIterationsAction.fromParentVariantMap(variantMap);
    _updateIterationsAction.fromParentVariantMap(variantMap);
    _computationAction.fromParentVariantMap(variantMap);

}

QVariantMap HsneScaleAction::toVariantMap() const
{
    QVariantMap variantMap = GroupAction::toVariantMap();

    _refineAction.insertIntoVariantMap(variantMap);
    _numIterationsAction.insertIntoVariantMap(variantMap);
    _numberOfComputatedIterationsAction.insertIntoVariantMap(variantMap);
    _updateIterationsAction.insertIntoVariantMap(variantMap);
    _computationAction.insertIntoVariantMap(variantMap);

    variantMap.insert({ { "inputGUID", QVariant::fromValue(_input.get<Points>()->getId()) } });
    variantMap.insert({ { "embeddingGUID", QVariant::fromValue(_embedding.get<Points>()->getId()) } });
    
    QString refineEmbeddingGUID = "";
    if(_refineEmbedding.isValid())
        refineEmbeddingGUID = _refineEmbedding.get<Points>()->getId();
    
    variantMap.insert({ { "refineEmbeddingGUID", QVariant::fromValue(refineEmbeddingGUID) } });

    variantMap["drillIndices"] = rawDataToVariantMap((char*)_drillIndices.data(), _drillIndices.size() * sizeof(uint32_t), true);
    variantMap.insert({ { "isTopScale", QVariant::fromValue(_isTopScale) } });
    variantMap.insert({ { "currentScaleLevel", QVariant::fromValue(_currentScaleLevel) } });

    return variantMap;
}
