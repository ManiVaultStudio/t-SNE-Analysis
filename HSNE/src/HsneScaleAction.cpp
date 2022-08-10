#include "HsneScaleAction.h"
#include "HsneHierarchy.h"
#include "TsneSettingsAction.h"
#include "DataHierarchyItem.h"

#include <QGridLayout>
#include <QMenu>

using namespace hdps;
using namespace hdps::gui;

CoreInterface* HsneScaleAction::core = nullptr;

HsneScaleAction::HsneScaleAction(QObject* parent, TsneSettingsAction& tsneSettingsAction, HsneHierarchy& hsneHierarchy, Dataset<Points> inputDataset, Dataset<Points> embeddingDataset) :
    GroupAction(parent, true),
    _tsneSettingsAction(tsneSettingsAction),
    _tsneAnalysis(),
    _hsneHierarchy(hsneHierarchy),
    _input(inputDataset),
    _embedding(embeddingDataset),
    _refineEmbedding(),
    _refineAction(this, "Refine..."),
    _isTopScale(true)
{
    setText("HSNE scale");

    _refineAction.setToolTip("Refine the selected landmarks");

    connect(&_refineAction, &TriggerAction::triggered, this, [this]() {
        refine();
    });

    _eventListener.setEventCore(core);

    const auto updateReadOnly = [this]() -> void {
        auto selection = _input->getSelection<Points>();

        _refineAction.setEnabled(!isReadOnly() && !selection->indices.empty());
    };

    connect(this, &GroupAction::readOnlyChanged, this, [this, updateReadOnly](const bool& readOnly) {
        updateReadOnly();
    });

    _eventListener.addSupportedEventType(static_cast<std::uint32_t>(EventType::DataSelectionChanged));
    _eventListener.registerDataEventByType(PointType, [this, updateReadOnly](DataEvent* dataEvent) {
        if (dataEvent->getDataset() == _embedding && dataEvent->getType() == EventType::DataSelectionChanged)
            updateReadOnly();
    });

    updateReadOnly();

    connect(&_tsneAnalysis, &TsneAnalysis::progressPercentage, this, [this](const float& percentage) {
        _refineEmbedding->getDataHierarchyItem().setTaskProgress(percentage);
    });

    connect(&_tsneAnalysis, &TsneAnalysis::progressSection, this, [this](const QString& section) {
        _refineEmbedding->getDataHierarchyItem().setTaskDescription(section);
    });

    connect(&_tsneAnalysis, &TsneAnalysis::finished, this, [this]() {
        _refineEmbedding->getDataHierarchyItem().setTaskFinished();
    });
}

QMenu* HsneScaleAction::getContextMenu(QWidget* parent /*= nullptr*/)
{
    auto menu = new QMenu(text(), parent);

    menu->addAction(&_refineAction);

    return menu;
}

void HsneScaleAction::refine()
{
    // Get associated points selection with embedding
    auto selection = _embedding->getSelection<Points>();

    // Get the refined scale level
    const auto refinedScaleLevel = _currentScaleLevel - 1;

    // Find proper selection indices
    std::vector<bool> selectedLocalIndices;
    _embedding->selectedLocalIndices(selection->indices, selectedLocalIndices);

    // Transform local indices to scale relative indices
    std::vector<unsigned int> selectionIndices; // Selected indices relative to scale
    for (int i = 0; i < selectedLocalIndices.size(); i++)
    {
        if (selectedLocalIndices[i])
        {
            selectionIndices.push_back(_isTopScale ? i : _drillIndices[i]);
        }
    }
    
    // Find the points in the previous level corresponding to selected landmarks
    std::map<uint32_t, float> neighbors;
    _hsneHierarchy.getInfluencedLandmarksInPreviousScale(_currentScaleLevel, selectionIndices, neighbors);

    // Threshold neighbours with enough influence
    std::vector<uint32_t> nextLevelIdxs; // Scale-relative indices
    nextLevelIdxs.clear();
    for (auto n : neighbors) {
        if (n.second > 0.5) //QUICKPAPER
        {
            nextLevelIdxs.push_back(n.first);
        }
    }
    std::cout << "#selected indices: " << selectionIndices.size() << std::endl;
    std::cout << "#landmarks at previous scale: " << neighbors.size() << std::endl;
    std::cout << "#thresholded at previous scale: " << nextLevelIdxs.size() << std::endl;
    std::cout << "Refining embedding.." << std::endl;
    
    // Compute the transition matrix for the landmarks above the threshold
    HsneMatrix transitionMatrix;
    _hsneHierarchy.getTransitionMatrixForSelection(_currentScaleLevel, transitionMatrix, nextLevelIdxs);

    // Create a new data set for the embedding
    {
        auto selection = _input->getSelection<Points>();

        Hsne::scale_type& refinedScale = _hsneHierarchy.getScale(refinedScaleLevel);

        selection->indices.clear();

        if (_input->isFull())
        {
            for (int i = 0; i < nextLevelIdxs.size(); i++)
                selection->indices.push_back(refinedScale._landmark_to_original_data_idx[nextLevelIdxs[i]]);
        }
        else
        {
            std::vector<unsigned int> globalIndices;
            _input->getGlobalIndices(globalIndices);
            for (int i = 0; i < nextLevelIdxs.size(); i++)
                selection->indices.push_back(globalIndices[refinedScale._landmark_to_original_data_idx[nextLevelIdxs[i]]]);
        }

        // Create HSNE scale subset
        auto hsneScaleSubset = _input->createSubsetFromSelection("hsne_scale", _input, false);

        // And the derived data for the embedding
        _refineEmbedding = core->createDerivedDataset<Points>(QString("%1_embedding").arg(_input->getGuiName()), hsneScaleSubset, _embedding);

        _refineEmbedding->setGuiName("HSNE Scale");
        _refineEmbedding->getDataHierarchyItem().select();
    }

    _refineEmbedding->setData(nullptr, 0, 2);

    // Only add a new scale action if the drill scale is higher than data level
    if (refinedScaleLevel > 0)
    {
        auto hsneScaleAction = new HsneScaleAction(this, _tsneSettingsAction, _hsneHierarchy, _input, _refineEmbedding);
        hsneScaleAction->setDrillIndices(nextLevelIdxs);
        hsneScaleAction->setScale(refinedScaleLevel);

        _refineEmbedding->addAction(*hsneScaleAction);
    }

    core->notifyDatasetAdded(_refineEmbedding);

    // Add linked selection between the upper embedding and the refined embedding
    if (refinedScaleLevel > 0)
    {
        LandmarkMap& landmarkMap = _hsneHierarchy.getInfluenceHierarchy().getMap()[_currentScaleLevel];

        auto selection = _embedding->getSelection<Points>();

        std::vector<unsigned int> localSelectionIndices;
        _embedding->getLocalSelectionIndices(localSelectionIndices);

        // Transmute local indices by drill indices specifying relation to full hierarchy scale
        if (!_isTopScale)
        {
            for (int i = 0; i < localSelectionIndices.size(); i++)
                localSelectionIndices[i] = _drillIndices[localSelectionIndices[i]];
        }

        hdps::SelectionMap mapping;

        if (_input->isFull())
        {
            for (const unsigned int& selectionIndex : localSelectionIndices)
            {
                int bottomLevelIdx = _hsneHierarchy.getScale(_currentScaleLevel)._landmark_to_original_data_idx[selectionIndex];
                mapping[bottomLevelIdx] = landmarkMap[selectionIndex];
            }
        }
        else
        {
            std::vector<unsigned int> globalIndices;
            _input->getGlobalIndices(globalIndices);
            for (const unsigned int& selectionIndex : localSelectionIndices)
            {
                std::vector<unsigned int> bottomMap = landmarkMap[selectionIndex];
                for (int j = 0; j < bottomMap.size(); j++)
                {
                    bottomMap[j] = globalIndices[bottomMap[j]];
                }
                int bottomLevelIdx = _hsneHierarchy.getScale(_currentScaleLevel)._landmark_to_original_data_idx[selectionIndex];
                mapping[globalIndices[bottomLevelIdx]] = bottomMap;
            }
        }

        _refineEmbedding->addLinkedData(_refineEmbedding, mapping);
    }

    _refineEmbedding->getDataHierarchyItem().setTaskName("HSNE scale");
    _refineEmbedding->getDataHierarchyItem().select();

    // Update embedding points when the TSNE analysis produces new data
    connect(&_tsneAnalysis, &TsneAnalysis::embeddingUpdate, this, [this](const TsneData& tsneData) {

        // Update the refine embedding with new data
        _refineEmbedding->setData(tsneData.getData().data(), tsneData.getNumPoints(), 2);

        // Notify others that the embedding points have changed
        core->notifyDatasetChanged(_refineEmbedding);
    });

    // Start the embedding process
    _tsneAnalysis.startComputation(_tsneSettingsAction.getTsneParameters(), transitionMatrix, nextLevelIdxs.size(), _hsneHierarchy.getNumDimensions());
}
