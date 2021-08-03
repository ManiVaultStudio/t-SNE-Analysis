#include "HsneAnalysisPlugin.h"

#include "PointData.h"
#include "HsneParameters.h"
#include "HsneScaleAction.h"

#include <QtCore>
#include <QtDebug>

Q_PLUGIN_METADATA(IID "nl.tudelft.HsneAnalysisPlugin")
#ifdef _WIN32
#include <windows.h>
#endif

#include <set>

#include <QMenu>

using namespace hdps;

HsneAnalysisPlugin::HsneAnalysisPlugin() :
    AnalysisPlugin("H-SNE Analysis"),
    _hsneSettingsAction(this)
{
    _hsneSettingsAction.getTsneSettingsAction().getGeneralTsneSettingsAction().collapse();
}

HsneAnalysisPlugin::~HsneAnalysisPlugin()
{
}

void HsneAnalysisPlugin::init()
{
    _outputDatasetName = _core->createDerivedData(QString("%1_embedding").arg(_inputDatasetName), _inputDatasetName);

    auto& inputDataset = _core->requestData<Points>(_inputDatasetName);
    auto& outputDataset = _core->requestData<Points>(_outputDatasetName);

    std::vector<float> initialData;

    const auto numEmbeddingDimensions = 2;

    initialData.resize(inputDataset.getNumPoints() * numEmbeddingDimensions);

    outputDataset.setData(initialData.data(), inputDataset.getNumPoints(), numEmbeddingDimensions);
    outputDataset.setParentDatasetName(_inputDatasetName);

    auto& tsneSettingsAction = _hsneSettingsAction.getTsneSettingsAction();

    outputDataset.exposeAction(&_hsneSettingsAction.getGeneralHsneSettingsAction());
    outputDataset.exposeAction(&_hsneSettingsAction.getAdvancedHsneSettingsAction());
    outputDataset.exposeAction(&tsneSettingsAction.getGeneralTsneSettingsAction());
    outputDataset.exposeAction(&tsneSettingsAction.getAdvancedTsneSettingsAction());
    outputDataset.exposeAction(&_hsneSettingsAction.getDimensionSelectionAction());
    outputDataset.exposeAction(new HsneScaleAction(this, &tsneSettingsAction, _core, &_hierarchy, _inputDatasetName, _outputDatasetName));

    connect(&_hsneSettingsAction.getStartStopAction(), &TriggerAction::toggled, this, [this](bool toggled) {
        if (toggled) {
            //_hsneSettingsAction.getGeneralHsneSettingsAction().setReadOnly(true);
            startComputation();
            //_hsneSettingsAction.getGeneralHsneSettingsAction().setReadOnly(false);
        }
            
    });

    registerDataEventByType(PointType, [this](hdps::DataEvent* dataEvent)
    {
        switch (dataEvent->getType())
        {
            case EventType::DataChanged:
            {
                // If we are not looking at the changed dataset, ignore it
                if (dataEvent->dataSetName != _inputDatasetName)
                    break;

                // Passes changes to the current dataset to the dimension selection widget
                Points& points = _core->requestData<Points>(dataEvent->dataSetName);

                _hsneSettingsAction.getDimensionSelectionAction().dataChanged(points);
                break;
            }
        }
    });

    _hsneSettingsAction.getDimensionSelectionAction().dataChanged(inputDataset);

    //connect(&_tsne, &TsneAnalysis::finished, _settings.get(), &HsneSettingsWidget::onComputationStopped);
    connect(&_tsne, &TsneAnalysis::embeddingUpdate, this, &HsneAnalysisPlugin::onNewEmbedding);
}

void HsneAnalysisPlugin::startComputation()
{
    notifyStarted();
    notifyProgressPercentage(0.0f);
    notifyProgressSection("Preparing data");
    
    // Obtain a reference to the the input dataset
    const Points& inputData = _core->requestData<Points>(_inputDatasetName);

    std::vector<bool> enabledDimensions = _hsneSettingsAction.getDimensionSelectionAction().getEnabledDimensions();

    notifyProgressSection("Initializing hierarchy");

    // Initialize the HSNE algorithm with the given parameters
    _hierarchy.initialize(_core, inputData, enabledDimensions, _hsneSettingsAction.getHsneParameters());

    notifyProgressSection("Computing top-level embedding");

    computeTopLevelEmbedding();
}

void HsneAnalysisPlugin::onNewEmbedding(const TsneData& tsneData) {
    Points& embedding = _core->requestData<Points>(_outputDatasetName);

    embedding.setData(tsneData.getData().data(), tsneData.getNumPoints(), 2);

    _core->notifyDataChanged(_outputDatasetName);
}

void HsneAnalysisPlugin::computeTopLevelEmbedding()
{
    // Get the top scale of the HSNE hierarchy
    int topScaleIndex = _hierarchy.getTopScale();
    Hsne::scale_type& topScale = _hierarchy.getScale(topScaleIndex);
    int numLandmarks = topScale.size();

    // Create a subset of the points corresponding to the top level HSNE landmarks,
    // Then create an empty embedding derived from this subset
    Points& inputData = _core->requestData<Points>(_inputDatasetName);
    Points& selection = static_cast<Points&>(inputData.getSelection());

    // Select the appropriate points to create a subset from
    selection.indices.resize(numLandmarks);
    for (int i = 0; i < numLandmarks; i++)
        selection.indices[i] = topScale._landmark_to_original_data_idx[i];

    // Create the subset and clear the selection
    QString subsetName = inputData.createSubset();
    selection.indices.clear();
    
    Points& embedding = _core->requestData<Points>(_outputDatasetName);

    embedding.setSourceData(subsetName);

    embedding.setProperty("scale", topScaleIndex);
    embedding.setProperty("landmarkMap", qVariantFromValue(_hierarchy.getInfluenceHierarchy().getMap()[topScaleIndex]));
    
    _hierarchy.printScaleInfo();

    // Set t-SNE parameters
    HsneParameters hsneParameters = _hsneSettingsAction.getHsneParameters();
    TsneParameters tsneParameters = _hsneSettingsAction.getTsneSettingsAction().getTsneParameters();

    // Embed data
    _tsne.stopComputation();
    _tsne.startComputation(tsneParameters, _hierarchy.getTransitionMatrixAtScale(topScaleIndex), numLandmarks, _hierarchy.getNumDimensions());
}

AnalysisPlugin* HsneAnalysisPluginFactory::produce()
{
    return new HsneAnalysisPlugin();
}

hdps::DataTypes HsneAnalysisPluginFactory::supportedDataTypes() const
{
    DataTypes supportedTypes;
    supportedTypes.append(PointType);
    return supportedTypes;
}
