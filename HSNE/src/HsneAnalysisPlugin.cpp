#include "HsneAnalysisPlugin.h"

#include "PointData.h"
#include "HsneParameters.h"

#include <QtCore>
#include <QtDebug>

Q_PLUGIN_METADATA(IID "nl.tudelft.HsneAnalysisPlugin")
#ifdef _WIN32
#include <windows.h>
#endif

#include <set>

#include <QMenu>

// =============================================================================
// View
// =============================================================================

using namespace hdps;

HsneAnalysisPlugin::HsneAnalysisPlugin() :
    AnalysisPlugin("H-SNE Analysis")
{

}

HsneAnalysisPlugin::~HsneAnalysisPlugin(void)
{
    
}

void HsneAnalysisPlugin::init()
{
    // Create a new settings widget which allows users to change the parameters given to the HSNE analysis
    _settings = std::make_unique<HsneSettingsWidget>(*this);

    // If a different input dataset is picked in the settings widget update the dimension widget
    connect(_settings.get(), &HsneSettingsWidget::dataSetPicked, this, &HsneAnalysisPlugin::dataSetPicked);
    // If the start computation button is pressed, run the HSNE algorithm
    connect(_settings.get(), &HsneSettingsWidget::startComputation, this, &HsneAnalysisPlugin::startComputation);

    registerDataEventByType(PointType, std::bind(&HsneAnalysisPlugin::onDataEvent, this, std::placeholders::_1));

    //connect(_settings.get(), &HsneSettingsWidget::stopComputation, this, &HsneAnalysisPlugin::stopComputation);
    connect(&_tsne, &TsneAnalysis::finished, _settings.get(), &HsneSettingsWidget::onComputationStopped);
    connect(&_tsne, &TsneAnalysis::embeddingUpdate, this, &HsneAnalysisPlugin::onNewEmbedding);
    //connect(&_tsne, SIGNAL(newEmbedding()), this, SLOT(onNewEmbedding()));
}

void HsneAnalysisPlugin::onDataEvent(hdps::DataEvent* dataEvent)
{
    switch (dataEvent->getType())
    {
    case EventType::DataAdded:
    {
        _settings->addDataItem(dataEvent->dataSetName);
        break;
    }
    case EventType::DataChanged:
    {
        // If we are not looking at the changed dataset, ignore it
        if (dataEvent->dataSetName != _settings->getCurrentDataItem()) {
            break;
        }

        // Passes changes to the current dataset to the dimension selection widget
        Points& points = _core->requestData<Points>(dataEvent->dataSetName);

        _settings->getDimensionSelectionWidget().dataChanged(points);
        break;
    }
    case EventType::DataRemoved:
    {
        _settings->removeDataItem(dataEvent->dataSetName);
        break;
    }
    }
}

SettingsWidget* const HsneAnalysisPlugin::getSettings()
{
    return _settings.get();
}

// If a different input dataset is picked in the settings widget update the dimension widget
void HsneAnalysisPlugin::dataSetPicked(const QString& name)
{
    Points& points = _core->requestData<Points>(name);

    auto analyses = points.getProperty("Analyses", QVariantList()).toList();
    analyses.push_back(getName());
    points.setProperty("Analyses", analyses);

    _settings->getDimensionSelectionWidget().dataChanged(points);
}

void HsneAnalysisPlugin::startComputation()
{
    //initializeTsne();

    /********************/
    /* Prepare the data */
    /********************/
    // Obtain a reference to the the input dataset
    QString setName = _settings->getCurrentDataItem();
    const Points& inputData = _core->requestData<Points>(setName);

    // Get the HSNE parameters from the settings widget
    HsneParameters parameters = _settings->getHsneParameters();

    std::vector<bool> enabledDimensions = _settings->getDimensionSelectionWidget().getEnabledDimensions();

    // Initialize the HSNE algorithm with the given parameters
    _hierarchy.initialize(_core, inputData, enabledDimensions, parameters);

    _embeddingNameBase = _settings->getEmbeddingName();
    _inputDataName = setName;
    computeTopLevelEmbedding();
}

void HsneAnalysisPlugin::onNewEmbedding(const TsneData& tsneData) {
    Points& embedding = _core->requestData<Points>(_embeddingName);

    embedding.setData(tsneData.getData().data(), tsneData.getNumPoints(), 2);

    _core->notifyDataChanged(_embeddingName);
}

QString HsneAnalysisPlugin::createEmptyDerivedEmbedding(QString name, QString dataType, QString sourceName)
{
    QString embeddingName = _core->createDerivedData(name, sourceName);
    Points& embedding = _core->requestData<Points>(embeddingName);
    embedding.setData(nullptr, 0, 2);

    auto analyses = embedding.getProperty("Analyses", QVariantList()).toList();
    analyses.push_back(getName());
    embedding.setProperty("Analyses", analyses);

    _core->notifyDataAdded(embeddingName);
    return embeddingName;
}

void HsneAnalysisPlugin::computeTopLevelEmbedding()
{
    // Get the top scale of the HSNE hierarchy
    int topScaleIndex = _hierarchy.getTopScale();
    Hsne::scale_type& topScale = _hierarchy.getScale(topScaleIndex);
    int numLandmarks = topScale.size();

    // Create a subset of the points corresponding to the top level HSNE landmarks,
    // Then create an empty embedding derived from this subset
    {
        Points& inputData = _core->requestData<Points>(_inputDataName);
        Points& selection = static_cast<Points&>(inputData.getSelection());

        // Select the appropriate points to create a subset from
        selection.indices.resize(numLandmarks);
        for (int i = 0; i < numLandmarks; i++)
            selection.indices[i] = topScale._landmark_to_original_data_idx[i];

        // Create the subset and clear the selection
        QString subsetName = inputData.createSubset();
        selection.indices.clear();

        // Create an empty embedding derived from the subset
        Points& subset = _core->requestData<Points>(subsetName);
        _embeddingName = createEmptyDerivedEmbedding(_embeddingNameBase + "_scale_" + QString::number(topScaleIndex), "Points", subsetName);
    }
    
    Points& embedding = _core->requestData<Points>(_embeddingName);
    embedding.setProperty("scale", topScaleIndex);
    embedding.setProperty("landmarkMap", qVariantFromValue(_hierarchy.getInfluenceHierarchy().getMap()[topScaleIndex]));
    
    _hierarchy.printScaleInfo();

    // Set t-SNE parameters
    HsneParameters hsneParameters = _settings->getHsneParameters();
    TsneParameters tsneParameters = _settings->getTsneParameters();

    // Embed data
    _tsne.stopComputation();
    _tsne.startComputation(tsneParameters, _hierarchy.getTransitionMatrixAtScale(topScaleIndex), numLandmarks, _hierarchy.getNumDimensions());
}

void HsneAnalysisPlugin::onDrillIn()
{
    //_hsne.drillIn("Temp");
}

void HsneAnalysisPlugin::drillIn(QString embeddingName)
{
    // Request the embedding from the core and find out the source data from which it derives
    Points& embedding = _core->requestData<Points>(embeddingName);
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
    _hierarchy.getInfluencedLandmarksInPreviousScale(currentScale, selectionIndices, neighbors);

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
    _hierarchy.getTransitionMatrixForSelection(currentScale, transitionMatrix, nextLevelIdxs);

    // Create a new data set for the embedding
    {
        Points& inputData = _core->requestData<Points>(_inputDataName);
        Points& selection = static_cast<Points&>(inputData.getSelection());
        Hsne::scale_type& dScale = _hierarchy.getScale(drillScale);

        //std::vector<unsigned int> dataIndices;
        selection.indices.clear();
        for (int i = 0; i < nextLevelIdxs.size(); i++)
        {
            selection.indices.push_back(dScale._landmark_to_original_data_idx[nextLevelIdxs[i]]);
        }
        
        QString subsetName = inputData.createSubset();
        Points& subset = _core->requestData<Points>(subsetName);

        _embeddingName = createEmptyDerivedEmbedding("Drill Embedding", "Points", subsetName);
    }

    // Store drill indices with embedding
    Points& drillEmbedding = _core->requestData<Points>(_embeddingName);
    QList<uint32_t> indices(nextLevelIdxs.begin(), nextLevelIdxs.end());
    QVariant variantIndices = QVariant::fromValue<QList<uint32_t>>(indices);
    drillEmbedding.setProperty("drill_indices", variantIndices);
    drillEmbedding.setProperty("scale", drillScale);
    drillEmbedding.setProperty("landmarkMap", qVariantFromValue(_hierarchy.getInfluenceHierarchy().getMap()[drillScale]));
    
    // Set t-SNE parameters
    TsneParameters tsneParameters = _settings->getTsneParameters();
    
    // Embed data
    _tsne.stopComputation();
    _tsne.startComputation(tsneParameters, transitionMatrix, nextLevelIdxs.size(), _hierarchy.getNumDimensions());
}

QMenu* HsneAnalysisPlugin::contextMenu(const QVariant& context)
{
    auto menu = new QMenu(getGuiName());

    auto startComputationAction = new QAction("Start computation");
    auto drillInAction = new QAction("Drill in");

    QMap contextMap = context.value<QMap<QString, QString>>();
    QString currentDataSetName = contextMap["CurrentDataset"];

    connect(startComputationAction, &QAction::triggered, [this]() { startComputation(); });
    connect(drillInAction, &QAction::triggered, [this, currentDataSetName]() { drillIn(currentDataSetName); });

    menu->addAction(startComputationAction);
    menu->addAction(drillInAction);

    return menu;
}

// =============================================================================
// Factory
// =============================================================================

AnalysisPlugin* HsneAnalysisPluginFactory::produce()
{
    return new HsneAnalysisPlugin();
}
