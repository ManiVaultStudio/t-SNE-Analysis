#include "HsneScaleAction.h"
#include "HsneHierarchy.h"
#include "TsneSettingsAction.h"
#include "DataHierarchyItem.h"

#include <QGridLayout>

using namespace hdps;
using namespace hdps::gui;

CoreInterface* HsneScaleAction::core = nullptr;

HsneScaleAction::HsneScaleAction(QObject* parent, TsneSettingsAction& tsneSettingsAction, HsneHierarchy& hsneHierarchy, const QString& inputDatasetName, const QString& embeddingDatasetName) :
    GroupAction(parent, true),
    EventListener(),
    _tsneSettingsAction(tsneSettingsAction),
    _tsneAnalysis(),
    _hsneHierarchy(hsneHierarchy),
    _input(inputDatasetName),
    _embedding(embeddingDatasetName),
    _refineEmbeddingName(),
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
        if (dataEvent->dataSetName != _embedding->getName())
            return;

        if (dataEvent->getType() == EventType::SelectionChanged)
            updateReadOnly();
    });

    updateReadOnly();

    connect(&_tsneAnalysis, &TsneAnalysis::progressPercentage, this, [this](const float& percentage) {
        core->getDataHierarchyItem(_refineEmbeddingName)->setTaskProgress(percentage);
    });

    connect(&_tsneAnalysis, &TsneAnalysis::progressSection, this, [this](const QString& section) {
        core->getDataHierarchyItem(_refineEmbeddingName)->setTaskDescription(section);
    });

    connect(&_tsneAnalysis, &TsneAnalysis::finished, this, [this]() {
        core->getDataHierarchyItem(_refineEmbeddingName)->setTaskFinished();
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

    const auto inputDatasetName = _input->getName();

    // Create a new data set for the embedding
    {
        auto& selection = static_cast<Points&>(_input->getSelection());
        Hsne::scale_type& dScale = _hsneHierarchy.getScale(drillScale);

        //std::vector<unsigned int> dataIndices;
        selection.indices.clear();
        for (int i = 0; i < nextLevelIdxs.size(); i++)
        {
            selection.indices.push_back(dScale._landmark_to_original_data_idx[nextLevelIdxs[i]]);
        }

        const auto subsetName = _input->createSubset("hsne_scale", false);

        _refineEmbeddingName = core->createDerivedData(QString("%1_embedding").arg(inputDatasetName), subsetName, _embedding->getName());
    }
    
    // Store drill indices with embedding
    auto& drillEmbedding = core->requestData<Points>(_refineEmbeddingName);

    drillEmbedding.setData(nullptr, 0, 2);

    auto hsneScaleAction = new HsneScaleAction(this, _tsneSettingsAction, _hsneHierarchy, inputDatasetName, _refineEmbeddingName);

    hsneScaleAction->setContext(drillEmbedding.getName());
    
    drillEmbedding.addAction(*hsneScaleAction);
    //drillEmbedding.exposeAction(&_tsneSettingsAction.getGeneralTsneSettingsAction());
    //drillEmbedding.exposeAction(&_tsneSettingsAction.getAdvancedTsneSettingsAction());

    core->notifyDataAdded(_refineEmbeddingName);

    QList<uint32_t> indices(nextLevelIdxs.begin(), nextLevelIdxs.end());
    QVariant variantIndices = QVariant::fromValue<QList<uint32_t>>(indices);
    drillEmbedding.setProperty("drill_indices", variantIndices);
    drillEmbedding.setProperty("scale", drillScale);
    drillEmbedding.setProperty("landmarkMap", qVariantFromValue(_hsneHierarchy.getInfluenceHierarchy().getMap()[drillScale]));
    
    auto refineEmbeddingDataHierarchyItem = core->getDataHierarchyItem(_refineEmbeddingName);

    refineEmbeddingDataHierarchyItem->setTaskName("HSNE scale");
    refineEmbeddingDataHierarchyItem->select();

    connect(&_tsneAnalysis, &TsneAnalysis::embeddingUpdate, this, [this](const TsneData& tsneData) {
        auto& embedding = core->requestData<Points>(_refineEmbeddingName);

        embedding.setData(tsneData.getData().data(), tsneData.getNumPoints(), 2);

        core->notifyDataChanged(_refineEmbeddingName);
    });

    // Start the embedding process
    _tsneAnalysis.startComputation(_tsneSettingsAction.getTsneParameters(), transitionMatrix, nextLevelIdxs.size(), _hsneHierarchy.getNumDimensions());
}
