#include "HsneAnalysisPlugin.h"
#include "HsneParameters.h"
#include "HsneScaleAction.h"

#include "PointData.h"

#include <QtCore>
#include <QtDebug>

Q_PLUGIN_METADATA(IID "nl.tudelft.HsneAnalysisPlugin")

#ifdef _WIN32
    #include <windows.h>
#endif

using namespace hdps;

HsneAnalysisPlugin::HsneAnalysisPlugin(const PluginFactory* factory) :
    AnalysisPlugin(factory),
    _hierarchy(),
    _tsneAnalysis(),
    _hsneSettingsAction(nullptr)
{
    
}

HsneAnalysisPlugin::~HsneAnalysisPlugin()
{
}

void HsneAnalysisPlugin::init()
{
    HsneScaleAction::core = _core;

    // Created derived dataset for embedding
    setOutputDatasetName(_core->createDerivedData("hsne_embedding", getInputDatasetName()));

    // Create new HSNE settings actions
    _hsneSettingsAction = new HsneSettingsAction(this);

    // Collapse the TSNE settings by default
    _hsneSettingsAction->getTsneSettingsAction().getGeneralTsneSettingsAction().collapse();

    // Get input/output datasets
    auto& inputDataset  = getInputDataset<Points>();
    auto& outputDataset = getOutputDataset<Points>();

    std::vector<float> initialData;

    const auto numEmbeddingDimensions = 2;

    initialData.resize(inputDataset.getNumPoints() * numEmbeddingDimensions);

    outputDataset.setData(initialData.data(), inputDataset.getNumPoints(), numEmbeddingDimensions);

    auto& tsneSettingsAction = _hsneSettingsAction->getTsneSettingsAction();
    
    outputDataset.addAction(_hsneSettingsAction->getGeneralHsneSettingsAction());
    outputDataset.addAction(_hsneSettingsAction->getAdvancedHsneSettingsAction());
    outputDataset.addAction(_hsneSettingsAction->getTopLevelScaleAction());
    outputDataset.addAction(tsneSettingsAction.getGeneralTsneSettingsAction());
    outputDataset.addAction(tsneSettingsAction.getAdvancedTsneSettingsAction());
    outputDataset.addAction(_hsneSettingsAction->getDimensionSelectionAction());

    _core->getDataHierarchyItem(outputDataset.getName())->select();

    connect(&_tsneAnalysis, &TsneAnalysis::progressPercentage, this, [this](const float& percentage) {
        setTaskProgress(percentage);
    });

    connect(&_tsneAnalysis, &TsneAnalysis::progressSection, this, [this](const QString& section) {
        setTaskDescription(section);
    });

    connect(&_tsneAnalysis, &TsneAnalysis::finished, this, [this]() {
        setTaskFinished();

        _hsneSettingsAction->getGeneralHsneSettingsAction().getStartAction().setEnabled(false);
        _hsneSettingsAction->setReadOnly(false);
    });

    connect(&_hsneSettingsAction->getGeneralHsneSettingsAction().getStartAction(), &TriggerAction::triggered, this, [this](bool toggled) {
        _hsneSettingsAction->setReadOnly(true);
        
        setTaskRunning();
        setTaskProgress(0.0f);
        setTaskDescription("Initializing HSNE hierarchy");

        qApp->processEvents();

        std::vector<bool> enabledDimensions = _hsneSettingsAction->getDimensionSelectionAction().getEnabledDimensions();

        // Initialize the HSNE algorithm with the given parameters
        _hierarchy.initialize(_core, getInputDataset<Points>(), enabledDimensions, _hsneSettingsAction->getHsneParameters());

        setTaskDescription("Computing top-level embedding");

        qApp->processEvents();

        computeTopLevelEmbedding();
    });

    registerDataEventByType(PointType, [this](hdps::DataEvent* dataEvent)
    {
        switch (dataEvent->getType())
        {
            case EventType::DataChanged:
            {
                // If we are not looking at the changed dataset, ignore it
                if (dataEvent->dataSetName != getInputDatasetName())
                    break;

                // Passes changes to the current dataset to the dimension selection widget
                Points& points = _core->requestData<Points>(dataEvent->dataSetName);

                _hsneSettingsAction->getDimensionSelectionAction().dataChanged(points);
                break;
            }
        }
    });

    _hsneSettingsAction->getDimensionSelectionAction().dataChanged(inputDataset);

    connect(&_tsneAnalysis, &TsneAnalysis::embeddingUpdate, this, [this](const TsneData& tsneData) {
        auto& embedding = getOutputDataset<Points>();

        embedding.setData(tsneData.getData().data(), tsneData.getNumPoints(), 2);

        _core->notifyDataChanged(getOutputDatasetName());
    });

    setTaskName("HSNE");
}

void HsneAnalysisPlugin::computeTopLevelEmbedding()
{
    // Get the top scale of the HSNE hierarchy
    int topScaleIndex = _hierarchy.getTopScale();
    Hsne::scale_type& topScale = _hierarchy.getScale(topScaleIndex);
    int numLandmarks = topScale.size();

    // Create a subset of the points corresponding to the top level HSNE landmarks,
    // Then create an empty embedding derived from this subset
    Points& inputData = getInputDataset<Points>();
    Points& selection = static_cast<Points&>(inputData.getSelection());

    // Select the appropriate points to create a subset from
    selection.indices.resize(numLandmarks);
    for (int i = 0; i < numLandmarks; i++)
        selection.indices[i] = topScale._landmark_to_original_data_idx[i];

    // Create the subset and clear the selection
    QString subsetName = inputData.createSubset("", false);
    selection.indices.clear();
    
    Points& embedding = getOutputDataset<Points>();

    embedding.setSourceData(subsetName);

    embedding.setProperty("scale", topScaleIndex);
    embedding.setProperty("landmarkMap", qVariantFromValue(_hierarchy.getInfluenceHierarchy().getMap()[topScaleIndex]));
    
    _hierarchy.printScaleInfo();

    // Set t-SNE parameters
    HsneParameters hsneParameters = _hsneSettingsAction->getHsneParameters();
    TsneParameters tsneParameters = _hsneSettingsAction->getTsneSettingsAction().getTsneParameters();

    // Embed data
    _tsneAnalysis.stopComputation();
    _tsneAnalysis.startComputation(tsneParameters, _hierarchy.getTransitionMatrixAtScale(topScaleIndex), numLandmarks, _hierarchy.getNumDimensions());
}

QIcon HsneAnalysisPlugin::getIcon() const
{
    return QIcon(":/images/icon.png");
}

AnalysisPlugin* HsneAnalysisPluginFactory::produce()
{
    return new HsneAnalysisPlugin(this);
}

hdps::DataTypes HsneAnalysisPluginFactory::supportedDataTypes() const
{
    DataTypes supportedTypes;
    supportedTypes.append(PointType);
    return supportedTypes;
}
