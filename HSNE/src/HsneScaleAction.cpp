#include "HsneScaleAction.h"
#include "HsneHierarchy.h"
#include "TsneSettingsAction.h"
#include "DataHierarchyItem.h"

#include <QGridLayout>

using namespace hdps;
using namespace hdps::gui;

CoreInterface* HsneScaleAction::core = nullptr;

HsneScaleAction::HsneScaleAction(QObject* parent, TsneSettingsAction& tsneSettingsAction, HsneHierarchy& hsneHierarchy, Points& inputDataset, Points& embeddingDataset) :
    GroupAction(parent, true),
    EventListener(),
    _tsneSettingsAction(tsneSettingsAction),
    _tsneAnalysis(),
    _hsneHierarchy(hsneHierarchy),
    _input(inputDataset),
    _embedding(embeddingDataset),
    _refineEmbedding(),
    _refineAction(this, "Refine...")
{
    setText("HSNE scale");

    _refineAction.setToolTip("Refine the selected landmarks");

    connect(&_refineAction, &TriggerAction::triggered, this, [this]() {
        refine();
    });

    setEventCore(core);

    const auto updateReadOnly = [this]() -> void {
        auto& selection = dynamic_cast<Points&>(_input->getSelection());

        _refineAction.setEnabled(!isReadOnly() && !selection.indices.empty());
    };

    connect(this, &GroupAction::readOnlyChanged, this, [this, updateReadOnly](const bool& readOnly) {
        updateReadOnly();
    });

    registerDataEventByType(PointType, [this, updateReadOnly](DataEvent* dataEvent) {
        if (dataEvent->getDataset() == *_embedding && dataEvent->getType() == EventType::DataSelectionChanged)
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
    // Get associated selection with embedding
    auto& selection = static_cast<Points&>(_embedding->getSelection()); // Global selection
    
    // Scale the current embedding is a part of
    int currentScale = _embedding->getProperty("scale").value<int>();
    int drillScale = currentScale - 1;

    // Find proper selection indices
    std::vector<bool> pointsSelected;
    _embedding->selectedLocalIndices(selection.indices, pointsSelected);

    std::vector<unsigned int> selectionIndices; // Selected indices relative to scale
    if (_embedding->hasProperty("drill_indices"))
    {
        QList<uint32_t> drillIndices = _embedding->getProperty("drill_indices").value<QList<uint32_t>>();

        for (int i = 0; i < pointsSelected.size(); i++)
        {
            if (pointsSelected[i])
            {
                selectionIndices.push_back(drillIndices[i]);
            }
        }
    }
    else
    {
        //selectionIndices = selection.indices;
        for (int i = 0; i < pointsSelected.size(); i++)
        {
            if (pointsSelected[i])
            {
                selectionIndices.push_back(i);
            }
        }
    }
    
    // Find the points in the previous level corresponding to selected landmarks
    std::map<uint32_t, float> neighbors;
    
    _hsneHierarchy.getInfluencedLandmarksInPreviousScale(currentScale, selectionIndices, neighbors);

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
    std::cout << "Drilling in" << std::endl;
    
    // Compute the transition matrix for the landmarks above the threshold
    HsneMatrix transitionMatrix;
    _hsneHierarchy.getTransitionMatrixForSelection(currentScale, transitionMatrix, nextLevelIdxs);

    // Create a new data set for the embedding
    {
        auto& selection = static_cast<Points&>(_input->getSelection());
        Hsne::scale_type& dScale = _hsneHierarchy.getScale(drillScale);

        //std::vector<unsigned int> dataIndices;
        selection.indices.clear();
        
        for (int i = 0; i < nextLevelIdxs.size(); i++)
            selection.indices.push_back(dScale._landmark_to_original_data_idx[nextLevelIdxs[i]]);

        // Create HSNE scale subset
        auto& subset = _input->createSubset("hsne_scale", _input.get(), false);

        // And the derived data for the embedding
        _refineEmbedding.set(core->createDerivedData(QString("%1_embedding").arg(_input->getGuiName()), subset, _embedding.get()));
    }

    _refineEmbedding->setData(nullptr, 0, 2);

    auto hsneScaleAction = new HsneScaleAction(this, _tsneSettingsAction, _hsneHierarchy, *_input, *_refineEmbedding);

    _refineEmbedding->addAction(*hsneScaleAction);

    core->notifyDataAdded(*_refineEmbedding);

    QList<uint32_t> indices(nextLevelIdxs.begin(), nextLevelIdxs.end());
    QVariant variantIndices = QVariant::fromValue<QList<uint32_t>>(indices);
    
    _refineEmbedding->setProperty("drill_indices", variantIndices);
    _refineEmbedding->setProperty("scale", drillScale);
    _refineEmbedding->setProperty("landmarkMap", qVariantFromValue(_hsneHierarchy.getInfluenceHierarchy().getMap()[drillScale]));
    
    // Add linked selection between the upper embedding and the refined embedding
    {
        std::vector<std::vector<unsigned int>> landmarkMap = _embedding->getProperty("landmarkMap").value<std::vector<std::vector<unsigned int>>>();

        Points& selection = static_cast<Points&>(_embedding->getSelection());

        std::vector<unsigned int> localSelectionIndices;
        _embedding->getLocalSelectionIndices(localSelectionIndices);

        // Transmute local indices by drill indices specifying relation to full hierarchy scale
        if (_embedding->hasProperty("drill_indices"))
        {
            QList<uint32_t> drillIndices = _embedding->getProperty("drill_indices").value<QList<uint32_t>>();

            for (int i = 0; i < localSelectionIndices.size(); i++)
                localSelectionIndices[i] = drillIndices[localSelectionIndices[i]];
        }

        hdps::SelectionMap mapping;
        for (const unsigned int& selectionIndex : localSelectionIndices)
        {
            int bottomLevelIdx = _hsneHierarchy.getScale(currentScale)._landmark_to_original_data_idx[selectionIndex];
            mapping[bottomLevelIdx] = landmarkMap[selectionIndex];
        }
        _embedding->addLinkedSelection(*_refineEmbedding, mapping);
    }

    _refineEmbedding->getDataHierarchyItem().setTaskName("HSNE scale");
    _refineEmbedding->getDataHierarchyItem().select();

    // Update embedding points when the TSNE analysis produces new data
    connect(&_tsneAnalysis, &TsneAnalysis::embeddingUpdate, this, [this](const TsneData& tsneData) {

        // Update the refine embedding with new data
        _refineEmbedding->setData(tsneData.getData().data(), tsneData.getNumPoints(), 2);

        // Notify others that the embedding points have changed
        core->notifyDataChanged(*_refineEmbedding);
    });

    // Start the embedding process
    _tsneAnalysis.startComputation(_tsneSettingsAction.getTsneParameters(), transitionMatrix, nextLevelIdxs.size(), _hsneHierarchy.getNumDimensions());
}
