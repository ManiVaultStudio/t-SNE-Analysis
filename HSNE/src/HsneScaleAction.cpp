#include "HsneScaleAction.h"
#include "HsneHierarchy.h"
#include "TsneSettingsAction.h"
#include "DataHierarchyItem.h"
#include "PointData.h"

using namespace hdps;
using namespace hdps::gui;

hdps::CoreInterface* HsneScaleAction::core = nullptr;

HsneScaleAction::HsneScaleAction(QObject* parent, TsneSettingsAction& tsneSettingsAction, HsneHierarchy& hsneHierarchy, hdps::DataHierarchyItem* inputDataHierarchyItem, hdps::DataHierarchyItem* embeddingDataHierarchyItem) :
    WidgetActionGroup(parent, true),
    hdps::EventListener(),
    _tsneSettingsAction(tsneSettingsAction),
    _tsneAnalysis(),
    _hsneHierarchy(hsneHierarchy),
    _inputDataHierarchyItem(inputDataHierarchyItem),
    _embeddingDataHierarchyItem(embeddingDataHierarchyItem),
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
        auto& dataset   = _embeddingDataHierarchyItem->getDataset<Points>();
        auto& selection = dynamic_cast<Points&>(dataset.getSelection());

        _refineAction.setEnabled(!isReadOnly() && !selection.indices.empty());
    };

    connect(this, &WidgetActionGroup::readOnlyChanged, this, [this, updateReadOnly](const bool& readOnly) {
        updateReadOnly();
    });

    registerDataEventByType(PointType, [this, updateReadOnly](hdps::DataEvent* dataEvent) {
        if (dataEvent->dataSetName != _embeddingDataHierarchyItem->getDatasetName())
            return;

        if (dataEvent->getType() == hdps::EventType::SelectionChanged)
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
    // Request the embedding from the core and find out the source data from which it derives
    auto& embedding = _embeddingDataHierarchyItem->getDataset<Points>();
    auto& source = hdps::DataSet::getSourceData<Points>(embedding);

    // Get associated selection with embedding
    auto& selection = static_cast<Points&>(embedding.getSelection());
    
    // Scale the embedding is a part of
    int currentScale = embedding.getProperty("scale").value<int>();
    int drillScale = currentScale - 1;

    // Find proper selection indices
    std::vector<bool> pointsSelected;
    embedding.selectedLocalIndices(selection.indices, pointsSelected);

    std::vector<unsigned int> selectionIndices;
    if (embedding.hasProperty("drill_indices"))
    {
        QList<uint32_t> drillIndices = embedding.getProperty("drill_indices").value<QList<uint32_t>>();

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
    std::vector<uint32_t> nextLevelIdxs;
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

    const auto inputDatasetName = _inputDataHierarchyItem->getDatasetName();

    // Create a new data set for the embedding
    {
        auto& inputData = _inputDataHierarchyItem->getDataset<Points>();
        auto& selection = static_cast<Points&>(inputData.getSelection());
        Hsne::scale_type& dScale = _hsneHierarchy.getScale(drillScale);

        //std::vector<unsigned int> dataIndices;
        selection.indices.clear();
        for (int i = 0; i < nextLevelIdxs.size(); i++)
        {
            selection.indices.push_back(dScale._landmark_to_original_data_idx[nextLevelIdxs[i]]);
        }

        const auto subsetName = inputData.createSubset("", false);
        auto& subset = core->requestData<Points>(subsetName);

        _refineEmbeddingName = core->createDerivedData(QString("%1_embedding").arg(inputDatasetName), inputDatasetName, _embeddingDataHierarchyItem->getDatasetName());

        auto& embedding = core->requestData<Points>(_refineEmbeddingName);
    }
    
    // Store drill indices with embedding
    auto& drillEmbedding = core->requestData<Points>(_refineEmbeddingName);

    drillEmbedding.setData(nullptr, 0, 2);
    drillEmbedding.exposeAction(new HsneScaleAction(this, _tsneSettingsAction, _hsneHierarchy, core->getDataHierarchyItem(inputDatasetName), core->getDataHierarchyItem(_refineEmbeddingName)));
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

HsneScaleAction::Widget::Widget(QWidget* parent, HsneScaleAction* hsneScaleAction, const Widget::State& state) :
    WidgetActionGroup::GroupWidget(parent, hsneScaleAction)
{
    layout()->setColumnStretch(0, 0);
    layout()->setColumnStretch(1, 0);

    layout()->addWidget(hsneScaleAction->getRefineAction().createWidget(this), 0, 0);
}
