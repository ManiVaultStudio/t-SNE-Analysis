#include "HsneScaleAction.h"
#include "HsneHierarchy.h"
#include "TsneSettingsAction.h"
#include "DataHierarchyItem.h"

#include <QGridLayout>
#include <QMenu>

using namespace mv;
using namespace mv::gui;

CoreInterface* HsneScaleAction::core = nullptr;

HsneScaleAction::HsneScaleAction(QObject* parent, TsneSettingsAction& tsneSettingsAction, HsneHierarchy& hsneHierarchy, Dataset<Points> inputDataset, Dataset<Points> embeddingDataset) :
    GroupAction(parent, "HSNE scale", true),
    _tsneSettingsAction(tsneSettingsAction),
    _tsneAnalysis(),
    _hsneHierarchy(hsneHierarchy),
    _input(inputDataset),
    _embedding(embeddingDataset),
    _refineEmbedding(),
    _refineAction(this, "Refine..."),
    _datasetPickerAction(this, "Selection IDs"),
    _reloadDatasetsAction(this, "Reload dataset"),
    _setSelectionAction(this, "Set selection"),
    _isTopScale(true)
{
    addAction(&_refineAction);
    addAction(&_datasetPickerAction);
    addAction(&_reloadDatasetsAction);
    addAction(&_setSelectionAction);

    _refineAction.setToolTip("Refine the selected landmarks");

    connect(&_refineAction, &TriggerAction::triggered, this, [this]() {
        refine();
    });

    const auto updateReadOnly = [this]() -> void {
        auto selection = _input->getSelection<Points>();

        _refineAction.setEnabled(!isReadOnly() && !selection->indices.empty());
    };

    connect(this, &GroupAction::readOnlyChanged, this, [this, updateReadOnly](const bool& readOnly) {
        updateReadOnly();
    });

    _eventListener.addSupportedEventType(static_cast<std::uint32_t>(EventType::DatasetDataSelectionChanged));
    _eventListener.registerDataEventByType(PointType, [this, updateReadOnly](DatasetEvent* dataEvent) {
        if (dataEvent->getDataset() == _embedding && dataEvent->getType() == EventType::DatasetDataSelectionChanged)
            updateReadOnly();
    });

    updateReadOnly();

    /*
    // Connect the progress percentage of the t-SNE process to the data hierarchy item associated with this embedding
    connect(&_tsneAnalysis, &TsneAnalysis::progressPercentage, this, [this](const float& percentage) {
        _refineEmbedding->getDataHierarchyItem().setTaskProgress(percentage);
    });

    // Connect the progress description of the t-SNE process to the data hierarchy item associated with this embedding
    connect(&_tsneAnalysis, &TsneAnalysis::progressSection, this, [this](const QString& section) {
        _refineEmbedding->getDataHierarchyItem().setTaskDescription(section);
    });

    // Connect the finishing of the t-SNE process to the data hierarchy item associated with this embedding
    connect(&_tsneAnalysis, &TsneAnalysis::finished, this, [this]() {
        _refineEmbedding->getDataHierarchyItem().setTaskFinished();
    });
    */

    auto setDatasets = [this]() ->void {
        // Get unique identifier and gui names from all point data sets in the core
        auto dataSets = Application::core()->requestAllDataSets(QVector<mv::DataType> {PointType});

        // Assign found dataset(s)
        _datasetPickerAction.setDatasets(dataSets);

    };

    setDatasets();

    connect(&_reloadDatasetsAction, &TriggerAction::triggered, this, [this, setDatasets]() {
        setDatasets();
        });

    connect(&_setSelectionAction, &TriggerAction::triggered, this, [this]() {
        
        auto selectionDataset = Dataset<Points>(_datasetPickerAction.getCurrentDataset().get<Points>());

        std::vector<unsigned int> selectionIDs;
        selectionIDs.resize(selectionDataset->getNumPoints());
        selectionDataset->populateDataForDimensions < std::vector<unsigned int>, std::vector<unsigned int>>(selectionIDs, std::vector<unsigned int>{ 0 });

        //_embedding->setSelectionIndices(selectionIDs);
        //events().notifyDatasetSelectionChanged(_embedding->getSourceDataset<Points>());
        _input->setSelectionIndices(selectionIDs);
        events().notifyDatasetDataSelectionChanged(_input->getSourceDataset<Points>());

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
    for (auto n : neighbors) {
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
        auto hsneScaleSubset = _input->createSubsetFromSelection("hsne_scale", _input, false);

        // And the derived data for the embedding
        _refineEmbedding = core->createDerivedDataset<Points>(QString("%1_embedding").arg(_input->text()), hsneScaleSubset, _embedding);

        _refineEmbedding->setText("HSNE Scale");
        _refineEmbedding->getDataHierarchyItem().select();
    }

    _refineEmbedding->setData(nullptr, 0, 2);

    auto hsneScaleSubset = _refineEmbedding->getSourceDataset<Points>();

    // Only add a new scale action if the drill scale is higher than data level
    if (refinedScaleLevel > 0)
    {
        auto hsneScaleAction = new HsneScaleAction(this, _tsneSettingsAction, _hsneHierarchy, _input, _refineEmbedding);
        hsneScaleAction->setDrillIndices(refinedLandmarks);
        hsneScaleAction->setScale(refinedScaleLevel);

        _refineEmbedding->addAction(*hsneScaleAction);
    }

    events().notifyDatasetAdded(_refineEmbedding);

    ///////////////////////////////////
    // Connect scales by linked data //
    ///////////////////////////////////
    
    // Add linked selection between the refined embedding and the bottom level points
    if (refinedScaleLevel > 0) // Only add a linked selection if it's not the bottom level already
    {
        LandmarkMap& landmarkMap = _hsneHierarchy.getInfluenceHierarchy().getMap()[refinedScaleLevel];

        mv::SelectionMap mapping;

        if (_input->isFull())
        {
            for (const unsigned int& scaleIndex : refinedLandmarks)
            {
                int bottomLevelIdx = _hsneHierarchy.getScale(refinedScaleLevel)._landmark_to_original_data_idx[scaleIndex];
                mapping.getMap()[bottomLevelIdx] = landmarkMap[scaleIndex];
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
                mapping.getMap()[globalIndices[bottomLevelIdx]] = bottomMap;
            }
        }

        _refineEmbedding->addLinkedData(_input, mapping);
    }

    _refineEmbedding->getTask().setName("HSNE scale");
    _refineEmbedding->getDataHierarchyItem().select();

    // Update embedding points when the TSNE analysis produces new data
    connect(&_tsneAnalysis, &TsneAnalysis::embeddingUpdate, this, [this](const TsneData& tsneData) {

        // Update the refine embedding with new data
        _refineEmbedding->setData(tsneData.getData().data(), tsneData.getNumPoints(), 2);

        // Notify others that the embedding points have changed
        events().notifyDatasetDataChanged(_refineEmbedding);
    });

    // Start the embedding process
    _tsneAnalysis.startComputation(_tsneSettingsAction.getTsneParameters(), transitionMatrix, refinedLandmarks.size(), _hsneHierarchy.getNumDimensions());
}
