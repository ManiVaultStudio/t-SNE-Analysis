#include "HsneScaleAction.h"

#include "DataHierarchyItem.h"
#include "GradientDescentSettingsAction.h"
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


HsneScaleAction::HsneScaleAction(QObject* parent, HsneHierarchy& hsneHierarchy, Dataset<Points> inputDataset, Dataset<Points> embeddingDataset) :
    GroupAction(parent, "HSNE Scale", true),
    _tsneParameters(),
    _tsneAnalysis(),
    _hsneHierarchy(hsneHierarchy),
    _input(inputDataset),
    _embedding(embeddingDataset),
    _refineEmbeddings(),
    _selectionHelpers(),
    _refineAction(this, "Refine selection"),
    _refinedScaledActions(),
    _computationAction(this, &_tsneParameters),
    _initializationTask(this, "Preparing HSNE scale"),
    _isTopScale(true),
    _currentScaleLevel(1),
    _tsneParametersTopLevel(nullptr)
{
}

HsneScaleAction::HsneScaleAction(QObject* parent, HsneHierarchy& hsneHierarchy, Dataset<Points> inputDataset, Dataset<Points> embeddingDataset, TsneParameters* tsneParametersTopLevel) :
    HsneScaleAction(parent, hsneHierarchy, inputDataset, embeddingDataset)
{
    _tsneParametersTopLevel = tsneParametersTopLevel;
    initLayoutAndConnection();
}

HsneScaleAction::HsneScaleAction(QObject* parent, HsneHierarchy& hsneHierarchy, Dataset<Points> inputDataset, Dataset<Points> embeddingDataset, unsigned int scale) :
    HsneScaleAction(parent, hsneHierarchy, inputDataset, embeddingDataset)
{
    _currentScaleLevel = scale;
    initLayoutAndConnection();
}

HsneScaleAction::~HsneScaleAction()
{
#ifdef HSNE_SCALE_ACTION_VERBOSE
    qDebug() << __FUNCTION__ << text();
#endif
}

void HsneScaleAction::initLayoutAndConnection()
{
    if (_currentScaleLevel > 0)
    {
        _refineAction.setToolTip("Refine the selected landmarks");
        addAction(&_refineAction);
        connect(&_refineAction, &TriggerAction::triggered, this, [this]() {
            refine();
            });
    }

    _computationAction.addActions();

    connect(&_computationAction.getNumIterationsAction(), &IntegralAction::valueChanged, this, [this](int32_t val) {
        _tsneParameters.setNumIterations(val);
        });

    // update number of iteration for top level embedding
    if (_tsneParametersTopLevel)
    {
        connect(&_computationAction.getNumIterationsAction(), &IntegralAction::valueChanged, this, [this](int32_t val) {
            _tsneParametersTopLevel->setNumIterations(val);
        });
    }
    // HSNE plugin takes care of recomputing the top level embedding
    else
    {
        const auto updateComputationAction = [this]() {
            const auto isRunning = _computationAction.getRunningAction().isChecked();

            _computationAction.getStartComputationAction().setEnabled(!isRunning);
            _computationAction.getContinueComputationAction().setEnabled(!isRunning && _tsneAnalysis.canContinue());
            _computationAction.getStopComputationAction().setEnabled(isRunning);
        };

        auto cleanupUpdateEmbedding = [this, updateComputationAction]() -> void {
            disconnect(&_tsneAnalysis, &TsneAnalysis::embeddingUpdate, nullptr, nullptr);
            _computationAction.getRunningAction().setChecked(false);
            setReadOnly(false);
            updateComputationAction();
        };

        connect(&_tsneAnalysis, &TsneAnalysis::finished, this, [this, cleanupUpdateEmbedding]() {
            cleanupUpdateEmbedding();
         });

        connect(&_tsneAnalysis, &TsneAnalysis::aborted, this, [this, cleanupUpdateEmbedding]() {
            cleanupUpdateEmbedding();
        });

        connect(&_tsneAnalysis, &TsneAnalysis::started, this, [this, updateComputationAction]() {
            _computationAction.getRunningAction().setChecked(true);
            updateComputationAction();
            qApp->processEvents();
        });

        auto initUpdateEmbedding = [this]() -> void {
            auto& datasetTask = _embedding->getTask();
            datasetTask.setName("Embed HSNE scale");
            datasetTask.setConfigurationFlag(Task::ConfigurationFlag::OverrideAggregateStatus);
            _tsneAnalysis.setTask(&datasetTask);
            datasetTask.setRunning();

            connect(&_tsneAnalysis, &TsneAnalysis::embeddingUpdate, this, [this](const TsneData& tsneData) {
                _embedding->setData(tsneData.getData().data(), tsneData.getNumPoints(), 2);
                getNumberOfComputatedIterationsAction().setValue(_tsneAnalysis.getNumIterations() - 1);
                events().notifyDatasetDataChanged(_embedding);
                });
        };

        connect(&_computationAction.getStartComputationAction(), &TriggerAction::triggered, this, [this, initUpdateEmbedding]() {
            initUpdateEmbedding();
            
            HsneMatrix refinedTransitionMatrix;
            assert(_currentScaleLevel + 1 <= _hsneHierarchy.getTopScale());
            _hsneHierarchy.getTransitionMatrixForSelection(_currentScaleLevel + 1, refinedTransitionMatrix, _drillIndices);

            assert(_drillIndices.size() == refinedTransitionMatrix.size());
            _tsneAnalysis.startComputation(_tsneParameters, refinedTransitionMatrix, _drillIndices.size());
        });

        connect(&_computationAction.getContinueComputationAction(), &TriggerAction::triggered, this, [this, initUpdateEmbedding]() {
            initUpdateEmbedding();

            _tsneAnalysis.continueComputation(_tsneParameters.getNumIterations());
        });

        connect(&_computationAction.getStopComputationAction(), &TriggerAction::triggered, this, [this]() {
            qApp->processEvents();
            _tsneAnalysis.stopComputation();
        });

    }

    const auto updateReadOnly = [this]() -> void {
        auto selection = _input->getSelection<Points>();
        const auto enabled = !isReadOnly();

        _refineAction.setEnabled(!isReadOnly() && !selection->indices.empty());
        _computationAction.getNumIterationsAction().setEnabled(enabled);
    };

    connect(this, &GroupAction::readOnlyChanged, this, [this, updateReadOnly](const bool& readOnly) {
        updateReadOnly();
    });

    _eventListener.addSupportedEventType(static_cast<std::uint32_t>(EventType::DatasetDataSelectionChanged));
    _eventListener.addSupportedEventType(static_cast<std::uint32_t>(EventType::DatasetAboutToBeRemoved));
    _eventListener.registerDataEventByType(PointType, [this, updateReadOnly](DatasetEvent* dataEvent) {
        const auto& removedData     = dataEvent->getDataset();
        const auto& removedDataID   = removedData->getId();

        if (dataEvent->getType() == EventType::DatasetDataSelectionChanged && removedDataID == _embedding->getId())
            updateReadOnly();

        //// Remove invisible selection helper dataset when scale dataset is removed
        //if (dataEvent->getType() == EventType::DatasetAboutToBeRemoved && removedData->hasProperty("selectionHelperID"))
        //{
        //    const auto& selectionHelperID = removedData->getProperty("selectionHelperID").toString();

        //    // Check if the removed dataset was a selection helper created by this scale
        //    auto wasCreatedByScale = [&selectionHelperID](Dataset<DatasetImpl> s) { return s->getId() == selectionHelperID; };
        //    if (auto it = std::find_if(std::begin(_selectionHelpers), std::end(_selectionHelpers), wasCreatedByScale); it != std::end(_selectionHelpers))
        //    {
        //        if ((*it).isValid())
        //        {
        //            qDebug() << "HSNE Scale: remove (invisible) selection helper dataset " << (*it)->getId() << " used for deleted " << removedDataID;
        //            mv::data().removeDataset(*it);
        //        }
        //        _selectionHelpers.erase(it);
        //    }
        //}

    });

    updateReadOnly();
}

QMenu* HsneScaleAction::getContextMenu(QWidget* parent /*= nullptr*/)
{
    auto menu = new QMenu(text(), parent);

    menu->addAction(&_refineAction);

    return menu;
}

void HsneScaleAction::initNonTopScale(const std::vector<uint32_t>& drillIndices)
{
    _drillIndices = drillIndices;
    _isTopScale = false;

    // Updates exxageration and exponential decay in _tsneParameters
    auto gradDescentAction = new GradientDescentSettingsAction(this, _tsneParameters);
    _embedding->addAction(*gradDescentAction);
}

void HsneScaleAction::refine()
{
    _initializationTask.setRunning();

    // Get the selection of points that are to be refined
    auto selection = _embedding->getSelection<Points>();

    // Set the scale of the refined embedding to be one below the current scale
    assert(_currentScaleLevel >= 1);
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
    HsneMatrix refinedTransitionMatrix;
    _hsneHierarchy.getTransitionMatrixForSelection(_currentScaleLevel, refinedTransitionMatrix, refinedLandmarks);

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

        // Create invisble subset from input data, used for selection mapping
        auto selectionHelperCount = _input->getProperty("selectionHelperCount").toInt();
        _input->setProperty("selectionHelperCount", ++selectionHelperCount);
        _selectionHelpers.push_back(_input->createSubsetFromSelection(QString("Hsne selection helper %1").arg(selectionHelperCount), _input, /* visible = */ false));
        auto& hsneScaleSubset = _selectionHelpers.back();

        // Create derived data for the embedding
        _refineEmbeddings.push_back(mv::data().createDerivedDataset<Points>(QString("Hsne scale %1").arg(refinedScaleLevel), hsneScaleSubset, _embedding));
        auto& refineEmbedding = _refineEmbeddings.back();
        refineEmbedding->setProperty("selectionHelperID", hsneScaleSubset->getId());

        //qDebug() << "refineEmbedding " << refineEmbedding->getId() << " with hsneScaleSubset " << hsneScaleSubset->getId();

        refineEmbedding->setData(nullptr, 0, 2);
        events().notifyDatasetDataChanged(refineEmbedding);

        // Handle data hierarchy item
        mv::dataHierarchy().clearSelection();
        refineEmbedding->getDataHierarchyItem().select();
        refineEmbedding->_infoAction->collapse();

        // Handle tasks
        auto& datasetTask = refineEmbedding->getTask();
        datasetTask.setName("HSNE scale computation");
        datasetTask.setConfigurationFlag(Task::ConfigurationFlag::OverrideAggregateStatus);

        _tsneAnalysis.setTask(&datasetTask);

        // Insert HsneScaleAction into new data set
        _refinedScaledActions.push_back(new HsneScaleAction(this, _hsneHierarchy, _input, refineEmbedding, refinedScaleLevel));
        auto& _refinedScaledAction = _refinedScaledActions.back();
        _refinedScaledAction->initNonTopScale(refinedLandmarks);

        refineEmbedding->addAction(*_refinedScaledAction);
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
    connect(&_tsneAnalysis, &TsneAnalysis::embeddingUpdate, this, [this, refinedScaleLevel](const TsneData& tsneData) {

        auto& refineEmbedding = _refineEmbeddings.back();

        // Update the refine embedding with new data
        refineEmbedding->setData(tsneData.getData().data(), tsneData.getNumPoints(), 2);

        _refinedScaledActions.back()->getNumberOfComputatedIterationsAction().setValue(_tsneAnalysis.getNumIterations() - 1);

        // Notify others that the embedding points have changed
        events().notifyDatasetDataChanged(refineEmbedding);
    });

    // Handle tasks
    _initializationTask.setFinished();

    auto& datasetTask = _refineEmbeddings.back()->getTask();
    datasetTask.setName("Embed HSNE scale");
    datasetTask.setConfigurationFlag(Task::ConfigurationFlag::OverrideAggregateStatus);
    _tsneAnalysis.setTask(&datasetTask);
    datasetTask.setRunning();

    // Get gradient descent settings from top level if applicable
    if (_isTopScale)
    {
        assert(_tsneParametersTopLevel != nullptr);
        _tsneParameters.setExaggerationIter(_tsneParametersTopLevel->getExaggerationIter());
        _tsneParameters.setExponentialDecayIter(_tsneParametersTopLevel->getExponentialDecayIter());
    }

    // Start the embedding process
    _tsneAnalysis.startComputation(_tsneParameters, refinedTransitionMatrix, refinedLandmarks.size());
}

void HsneScaleAction::fromVariantMap(const QVariantMap& variantMap)
{
    GroupAction::fromVariantMap(variantMap);

    // Handle data sets
    _input      = mv::data().getDataset(variantMap["inputGUID"].toString());
    _embedding  = mv::data().getDataset(variantMap["embeddingGUID"].toString());

    // Handle own data
    {
        const auto drillIndices = variantMap["drillIndices"].toMap();
        std::vector<uint32_t> drillIndicesVec;
        drillIndicesVec.resize(static_cast<size_t>(variantMap["drillIndicesSize"].toInt()));
        populateDataBufferFromVariantMap(drillIndices, (char*)drillIndicesVec.data());
        _drillIndices = std::move(drillIndicesVec);
    }

    _isTopScale         = variantMap["isTopScale"].toBool();
    _currentScaleLevel  = variantMap["currentScaleLevel"].toUInt();

    _refineAction.fromParentVariantMap(variantMap);
    _computationAction.fromParentVariantMap(variantMap);

    // Handle _tsneParameters
    _tsneParameters.setNumIterations(variantMap["NumIterations"].toInt());
    _tsneParameters.setExaggerationIter(variantMap["ExaggerationIter"].toInt());
    _tsneParameters.setExponentialDecayIter(variantMap["ExponentialDecayIter"].toInt());
    _tsneParameters.setNumDimensionsOutput(variantMap["NumDimensionsOutput"].toInt());
    _tsneParameters.setUpdateCore(variantMap["UpdateCore"].toInt());

    // Handle refined datasets and corresponding actions
    for (const auto& refinedEmbeddingMapVar : variantMap["refinedEmbeddingsMap"].toMap())
    {
        const auto refinedEmbeddingMap = refinedEmbeddingMapVar.toMap();
        const auto refineEmbeddingGUID = refinedEmbeddingMap["refineEmbeddingGUID"].toString();
        auto refineEmbedding = mv::data().getDataset<Points>(refineEmbeddingGUID);

        if (!refineEmbedding.isValid())
            continue;

        _refineEmbeddings.push_back(refineEmbedding);

        _refinedScaledActions.push_back(new HsneScaleAction(this, _hsneHierarchy, _input, refineEmbedding, refinedEmbeddingMap["refinedCurrentScaleLevel"].toUInt()));
        HsneScaleAction* refinedScaledAction = _refinedScaledActions.back();
        refinedScaledAction->fromParentVariantMap(refinedEmbeddingMap);

        refineEmbedding->addAction(*refinedScaledAction);
        refineEmbedding->_infoAction->collapse();
    }
    assert(_refineEmbeddings.size() == _refinedScaledActions.size());

    // Handle references to selection helpers
    for (const auto& selectionHelperGUID : variantMap["selectionHelpersList"].toStringList())
    {
        auto selectionHelper = mv::data().getDataset<Points>(selectionHelperGUID);
        if (selectionHelper.isValid())
            _selectionHelpers.push_back(selectionHelper);
    }
    assert(_refineEmbeddings.size() == _selectionHelpers.size());
}

QVariantMap HsneScaleAction::toVariantMap() const
{
    QVariantMap variantMap = GroupAction::toVariantMap();

    // Handle data sets
    variantMap["inputGUID"]     = QVariant::fromValue(_input.get<Points>()->getId());
    variantMap["embeddingGUID"] = QVariant::fromValue(_embedding.get<Points>()->getId());
    
    // Handle own data
    variantMap["drillIndices"]      = rawDataToVariantMap((char*)_drillIndices.data(), _drillIndices.size() * sizeof(uint32_t), true);
    variantMap["drillIndicesSize"]  = QVariant::fromValue(_drillIndices.size());
    variantMap["isTopScale"]        = QVariant::fromValue(_isTopScale);
    variantMap["currentScaleLevel"] = QVariant::fromValue(_currentScaleLevel);

    _refineAction.insertIntoVariantMap(variantMap);
    _computationAction.insertIntoVariantMap(variantMap);

    // Handle _tsneParameters
    variantMap["NumIterations"]         = QVariant::fromValue(_tsneParameters.getNumIterations());
    variantMap["ExaggerationIter"]      = QVariant::fromValue(_tsneParameters.getExaggerationIter());
    variantMap["ExponentialDecayIter"]  = QVariant::fromValue(_tsneParameters.getExponentialDecayIter());
    variantMap["NumDimensionsOutput"]   = QVariant::fromValue(_tsneParameters.getNumDimensionsOutput());
    variantMap["UpdateCore"]            = QVariant::fromValue(_tsneParameters.getUpdateCore());

    // Handle refined datasets and corresponding actions
    QVariantMap refinedEmbeddingsMap;

    assert(_refineEmbeddings.size() == _refinedScaledActions.size());
    for (size_t i = 0; i < _refineEmbeddings.size(); i++) {
        const auto& refineEmbedding                 = _refineEmbeddings[i];
        const HsneScaleAction* refinedScaledAction  = _refinedScaledActions[i];

        if (!refineEmbedding.isValid() || refinedScaledAction == nullptr)
            continue;

        QVariantMap refinedEmbeddingMap;
        refinedEmbeddingMap["refineEmbeddingGUID"]      = QVariant::fromValue(refineEmbedding.get<Points>()->getId());
        refinedEmbeddingMap["refinedCurrentScaleLevel"] = QVariant::fromValue(refinedScaledAction->_currentScaleLevel);
        refinedScaledAction->insertIntoVariantMap(refinedEmbeddingMap);
        refinedEmbeddingsMap[QString::number(i)]        = refinedEmbeddingMap;
    }

    variantMap["refinedEmbeddingsMap"]  = refinedEmbeddingsMap;

    // Handle references to selection helpers
    QStringList selectionHelperList;

    for (const auto& selectionHelper : _selectionHelpers) {
        if (selectionHelper.isValid())
            selectionHelperList.append(selectionHelper.get<Points>()->getId());
    }

    variantMap["selectionHelpersList"]  = QVariant::fromValue(selectionHelperList);

    return variantMap;
}
