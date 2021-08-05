#include "HsneScaleAction.h"
#include "HsneHierarchy.h"
#include "TsneSettingsAction.h"

#include "CoreInterface.h"
#include "PointData.h"

using namespace hdps::gui;

HsneScaleAction::HsneScaleAction(QObject* parent, TsneSettingsAction* tsneSettingsAction, hdps::CoreInterface* core, HsneHierarchy* hsneHierarchy, const QString& inputDataSetName, const QString& inputEmbeddingName) :
    WidgetActionGroup(parent, true),
    hdps::EventListener(),
    _tsneSettingsAction(tsneSettingsAction),
    _core(core),
    _tsne(),
    _hsneHierarchy(hsneHierarchy),
    _inputDatasetName(inputDataSetName),
    _inputEmbeddingName(inputEmbeddingName),
    _refineEmbeddingName(),
    _refineAction(this, "Refine...")
{
    setText("HSNE scale");

    connect(&_refineAction, &TriggerAction::triggered, this, [this]() {
        refine();
    });

    setEventCore(_core);

    registerDataEventByType(PointType, [this](hdps::DataEvent* dataEvent) {
        if (dataEvent->dataSetName != _inputEmbeddingName)
            return;

        switch (dataEvent->getType())
        {
            case hdps::EventType::SelectionChanged:
            {
                auto& embedding = dynamic_cast<Points&>(_core->requestData(_inputEmbeddingName).getSelection());
                _refineAction.setEnabled(!embedding.indices.empty());
                break;
            }

            default:
                break;
        }
    });
}

QMenu* HsneScaleAction::getContextMenu()
{
    auto menu = new QMenu(text());

    menu->addAction(&_refineAction);

    return menu;
}

void HsneScaleAction::refine()
{
    // Request the embedding from the core and find out the source data from which it derives
    Points& embedding = _core->requestData<Points>(_inputEmbeddingName);
    Points& source = hdps::DataSet::getSourceData<Points>(embedding);

    // Get associated selection with embedding
    Points& selection = static_cast<Points&>(embedding.getSelection());
    
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
    
    _hsneHierarchy->getInfluencedLandmarksInPreviousScale(currentScale, selectionIndices, neighbors);

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
    _hsneHierarchy->getTransitionMatrixForSelection(currentScale, transitionMatrix, nextLevelIdxs);

    // Create a new data set for the embedding
    {
        Points& inputData = _core->requestData<Points>(_inputDatasetName);
        Points& selection = static_cast<Points&>(inputData.getSelection());
        Hsne::scale_type& dScale = _hsneHierarchy->getScale(drillScale);

        //std::vector<unsigned int> dataIndices;
        selection.indices.clear();
        for (int i = 0; i < nextLevelIdxs.size(); i++)
        {
            selection.indices.push_back(dScale._landmark_to_original_data_idx[nextLevelIdxs[i]]);
        }

        QString subsetName = inputData.createSubset("", false);
        Points& subset = _core->requestData<Points>(subsetName);

        _refineEmbeddingName = _core->createDerivedData(QString("%1_embedding").arg(_inputDatasetName), _inputDatasetName, _inputEmbeddingName);

        Points& embedding = _core->requestData<Points>(_refineEmbeddingName);
        
    }
    
    // Store drill indices with embedding
    Points& drillEmbedding = _core->requestData<Points>(_refineEmbeddingName);

    drillEmbedding.setData(nullptr, 0, 2);
    drillEmbedding.exposeAction(new HsneScaleAction(this, _tsneSettingsAction, _core, _hsneHierarchy, _inputDatasetName, _refineEmbeddingName));
    drillEmbedding.exposeAction(&_tsneSettingsAction->getGeneralTsneSettingsAction());

    _core->notifyDataAdded(_refineEmbeddingName);

    QList<uint32_t> indices(nextLevelIdxs.begin(), nextLevelIdxs.end());
    QVariant variantIndices = QVariant::fromValue<QList<uint32_t>>(indices);
    drillEmbedding.setProperty("drill_indices", variantIndices);
    drillEmbedding.setProperty("scale", drillScale);
    drillEmbedding.setProperty("landmarkMap", qVariantFromValue(_hsneHierarchy->getInfluenceHierarchy().getMap()[drillScale]));
    
    _core->getDataHierarchyItem(drillEmbedding.getName()).select();

    connect(&_tsne, &TsneAnalysis::embeddingUpdate, this, [this](const TsneData& tsneData) {
        Points& embedding = _core->requestData<Points>(_refineEmbeddingName);

        embedding.setData(tsneData.getData().data(), tsneData.getNumPoints(), 2);

        _core->notifyDataChanged(_refineEmbeddingName);
    });

    // Set t-SNE parameters
    TsneParameters tsneParameters = _tsneSettingsAction->getTsneParameters();

    // Embed data
    _tsne.startComputation(tsneParameters, transitionMatrix, nextLevelIdxs.size(), _hsneHierarchy->getNumDimensions());
}

HsneScaleAction::Widget::Widget(QWidget* parent, HsneScaleAction* HsneScaleAction, const Widget::State& state) :
    WidgetAction::Widget(parent, HsneScaleAction, state)
{
}
