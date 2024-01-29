#include "HsneScaleAction.h"

#include "DataHierarchyItem.h"
#include "HsneHierarchy.h"
#include "TsneParameters.h"

#include <event/Event.h>

#include <PointData/InfoAction.h>

#include <limits>

#include <QMenu>

#ifdef _DEBUG
    #define HSNE_SCALE_ACTION_VERBOSE
#endif

using namespace mv;
using namespace mv::gui;

HsneScaleAction::HsneScaleAction(QObject* parent, TsneParameters& tsneParameters, HsneHierarchy& hsneHierarchy, Dataset<Points> inputDataset, Dataset<Points> embeddingDataset) :
    GroupAction(parent, "HSNE Scale", true),
    _tsneParameters(tsneParameters),
    _tsneAnalysis(),
    _hsneHierarchy(hsneHierarchy),
    _input(inputDataset),
    _embedding(embeddingDataset),
    _refineEmbeddings(),
    _refineAction(this, "Refine selection"),
    _numIterationsAction(this, "New iterations", 1, 10000, 1000),
    _numberOfComputatedIterationsAction(this, "Computed iterations", 0, std::numeric_limits<int>::max(), 0),
    _updateIterationsAction(this, "Core update every", 0, 10000, 10),
    _computationAction(this),
    _refinedScaledActions(),
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

    _refineAction.setToolTip("Refine the selected landmarks");
    _updateIterationsAction.setToolTip("Update the dataset every x iterations. If set to 0, there will be no intermediate result.");
    _numIterationsAction.setToolTip("Number of new iterations that will be computed when pressing start or continue.");
    _numberOfComputatedIterationsAction.setToolTip("Number of iterations that have already been computed.");

    _numberOfComputatedIterationsAction.setEnabled(false);

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
        //_updateIterationsAction.setEnabled(enabled);
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

HsneScaleAction::~HsneScaleAction()
{
#ifdef HSNE_SCALE_ACTION_VERBOSE
    qDebug() << __FUNCTION__ << text();
#endif
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
        _refineEmbeddings.push_back(mv::data().createDerivedDataset<Points>(QString("Hsne scale %1").arg(refinedScaleLevel), hsneScaleSubset, _embedding));
        auto& refineEmbedding = _refineEmbeddings.back();

        refineEmbedding->setData(nullptr, 0, 2);
        events().notifyDatasetDataChanged(refineEmbedding);

        mv::dataHierarchy().clearSelection();
        refineEmbedding->getDataHierarchyItem().select();
        refineEmbedding->_infoAction->collapse();

        auto& datasetTask = refineEmbedding->getTask();
        datasetTask.setName("HSNE scale computation");
        datasetTask.setConfigurationFlag(Task::ConfigurationFlag::OverrideAggregateStatus);

        _tsneAnalysis.setTask(&datasetTask);

        // Only add a new scale action if the drill scale is higher than data level
        if (refinedScaleLevel > 0)
        {
            _refinedScaledActions.push_back(new HsneScaleAction(this, _tsneParameters, _hsneHierarchy, _input, refineEmbedding));
            auto& _refinedScaledAction = _refinedScaledActions.back();
            _refinedScaledAction->setDrillIndices(refinedLandmarks);
            _refinedScaledAction->setScale(refinedScaleLevel);

            refineEmbedding->addAction(*_refinedScaledAction);
        }
        else
        {
            _refinedScaledActions.push_back(nullptr);

            auto dataScaleAction = new GroupAction(this, "HSNE Scale");
            auto refineNumIterationsAction = new IntegralAction(this, "New iterations", 1, 10000, 1000);
            auto refineNumberOfComputatedIterationsAction = new IntegralAction(this, "Computed iterations", 0, std::numeric_limits<int>::max(), 0);
            auto refinedComputeAction = new TsneComputationAction(this);

            refineNumIterationsAction->setDefaultWidgetFlags(IntegralAction::SpinBox);
            refineNumberOfComputatedIterationsAction->setDefaultWidgetFlags(IntegralAction::LineEdit);

            refineNumberOfComputatedIterationsAction->setEnabled(false);

            dataScaleAction->addAction(refineNumIterationsAction);
            dataScaleAction->addAction(refineNumberOfComputatedIterationsAction);
            dataScaleAction->addAction(refinedComputeAction);

            refineEmbedding->addAction(*dataScaleAction);

            connect(refinedComputeAction, &TriggerAction::triggered, this, [this]() {
                // TODO: recompute
                });

            const auto updateReadOnly = [this, refinedComputeAction, refineNumIterationsAction]() -> void {
                auto selection = _input->getSelection<Points>();
                const auto enabled = !isReadOnly();

                refinedComputeAction->setEnabled(!isReadOnly() && !selection->indices.empty());
                refineNumIterationsAction->setEnabled(enabled);
                };

            connect(this, &GroupAction::readOnlyChanged, this, [this, updateReadOnly](const bool& readOnly) {
                updateReadOnly();
                });

            connect(refineNumIterationsAction, &IntegralAction::valueChanged, this, [this, refineNumIterationsAction](const std::int32_t& value) {
                _tsneParameters.setNumIterations(refineNumIterationsAction->getValue());
                });

        }
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

        _refineEmbeddings.back()->addLinkedData(_input, mapping);
    }

    // Update embedding points when the TSNE analysis produces new data
    connect(&_tsneAnalysis, &TsneAnalysis::embeddingUpdate, this, [this](const TsneData& tsneData) {

        auto& refineEmbedding = _refineEmbeddings.back();

        // Update the refine embedding with new data
        refineEmbedding->setData(tsneData.getData().data(), tsneData.getNumPoints(), 2);

        // Notify others that the embedding points have changed
        events().notifyDatasetDataChanged(refineEmbedding);
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

    // Handle refined datasets and corresponding actions
    for (const auto& refinedEmbeddingMapVar : variantMap["refinedEmbeddingsMap"].toMap())
    {
        const auto refinedEmbeddingMap = refinedEmbeddingMapVar.toMap();
        const auto refineEmbeddingGUID = refinedEmbeddingMap["refineEmbeddingGUID"].toString();
        auto refineEmbedding = mv::data().getDataset<Points>(refineEmbeddingGUID);

        if (refineEmbedding.isValid())
        {
            _refineEmbeddings.push_back(refineEmbedding);

            if (refinedEmbeddingMap["refinedCurrentScaleLevel"].toUInt() > 0)
            {
                _refinedScaledActions.push_back(new HsneScaleAction(this, _tsneParameters, _hsneHierarchy, _input, refineEmbedding));
                HsneScaleAction* refinedScaledAction = _refinedScaledActions.back();

                const auto refinedDrillIndices = refinedEmbeddingMap["refinedDrillIndices"].toMap();
                std::vector<uint32_t> refinedDrillIndicesVec;
                refinedDrillIndicesVec.resize(static_cast<size_t>(refinedEmbeddingMap["refinedDrillIndicesSize"].toInt()));
                populateDataBufferFromVariantMap(refinedDrillIndices, (char*)refinedDrillIndicesVec.data());
                refinedScaledAction->setDrillIndices(std::move(refinedDrillIndicesVec));
                refinedScaledAction->setScale(refinedEmbeddingMap["refinedCurrentScaleLevel"].toUInt());    // sets _isTopScale = false

                refineEmbedding->addAction(*refinedScaledAction);
                refineEmbedding->_infoAction->collapse();
            }
            else
            {
                _refinedScaledActions.push_back(nullptr);
                auto refinedComputeAction = new TsneComputationAction(this);
                refineEmbedding->addAction(*refinedComputeAction);
            }

        }
    }
    assert(_refineEmbeddings.size() == _refinedScaledActions.size());

    // Handle own data
    {
        const auto drillIndices = variantMap["drillIndices"].toMap();
        std::vector<uint32_t> drillIndicesVec;
        drillIndicesVec.resize(static_cast<size_t>(variantMap["drillIndicesSize"].toInt()));
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

    variantMap["inputGUID"] = QVariant::fromValue(_input.get<Points>()->getId());
    variantMap["embeddingGUID"] = QVariant::fromValue(_embedding.get<Points>()->getId());
    
    QVariantMap refinedEmbeddingsMap;

    assert(_refineEmbeddings.size() == _refinedScaledActions.size());
    for (size_t i = 0; i < _refineEmbeddings.size(); i++) {
        const auto& refineEmbedding = _refineEmbeddings[i];
        const HsneScaleAction* refinedScaledAction = _refinedScaledActions[i];

        QVariantMap refinedEmbeddingMap;

        if (refineEmbedding.isValid() )
        {
            refinedEmbeddingMap["refineEmbeddingGUID"] = QVariant::fromValue(refineEmbedding.get<Points>()->getId());

            if (refinedScaledAction != nullptr)
            {
                refinedEmbeddingMap["refinedDrillIndices"] = rawDataToVariantMap((char*)refinedScaledAction->_drillIndices.data(), refinedScaledAction->_drillIndices.size() * sizeof(uint32_t), true);
                refinedEmbeddingMap["refinedDrillIndicesSize"] = QVariant::fromValue(refinedScaledAction->_drillIndices.size());
                refinedEmbeddingMap["refinedCurrentScaleLevel"] = QVariant::fromValue(refinedScaledAction->_currentScaleLevel);
            }
            else
                refinedEmbeddingMap["refinedCurrentScaleLevel"] = QVariant::fromValue(0);
        }

        refinedEmbeddingsMap[QString::number(i)] = refinedEmbeddingMap;
    }

    variantMap["refinedEmbeddingsMap"] = refinedEmbeddingsMap;

    variantMap["drillIndices"] = rawDataToVariantMap((char*)_drillIndices.data(), _drillIndices.size() * sizeof(uint32_t), true);
    variantMap["drillIndicesSize"] = QVariant::fromValue(_drillIndices.size());
    variantMap["isTopScale"] = QVariant::fromValue(_isTopScale);
    variantMap["currentScaleLevel"] = QVariant::fromValue(_currentScaleLevel);

    return variantMap;
}
