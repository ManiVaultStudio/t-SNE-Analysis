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

HsneAnalysisPlugin::HsneAnalysisPlugin() :
    AnalysisPlugin("H-SNE Analysis"),
    _hierarchy(),
    _tsneAnalysis(),
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

    auto& tsneSettingsAction = _hsneSettingsAction.getTsneSettingsAction();

    outputDataset.exposeAction(&_hsneSettingsAction.getGeneralHsneSettingsAction());
    outputDataset.exposeAction(&_hsneSettingsAction.getAdvancedHsneSettingsAction());
    outputDataset.exposeAction(new HsneScaleAction(this, &_hsneSettingsAction.getTsneSettingsAction(), _core, &_hierarchy, _inputDatasetName, _outputDatasetName));
    outputDataset.exposeAction(&tsneSettingsAction.getGeneralTsneSettingsAction());
    outputDataset.exposeAction(&tsneSettingsAction.getAdvancedTsneSettingsAction());
    outputDataset.exposeAction(&_hsneSettingsAction.getDimensionSelectionAction());

    _core->getDataHierarchyItem(outputDataset.getName()).select();

    connect(&_tsneAnalysis, &TsneAnalysis::progressPercentage, this, [this](const float& percentage) {
        notifyProgressPercentage(percentage);
    });

    connect(&_tsneAnalysis, &TsneAnalysis::progressSection, this, [this](const QString& section) {
        notifyProgressSection(section);
    });

    connect(&_tsneAnalysis, &TsneAnalysis::finished, this, [this]() {
        notifyFinished();
        notifyProgressPercentage(0.0f);
        notifyProgressSection("");

        _hsneSettingsAction.getStartAction().setEnabled(false);
    });

    connect(&_hsneSettingsAction.getStartAction(), &TriggerAction::triggered, this, [this]() {
        _hsneSettingsAction.getStartAction().setEnabled(false);

        notifyStarted();
        notifyProgressPercentage(0.0f);
        notifyProgressSection("Preparing HSNE data");

        // Obtain a reference to the the input dataset
        const Points& inputData = _core->requestData<Points>(_inputDatasetName);

        std::vector<bool> enabledDimensions = _hsneSettingsAction.getDimensionSelectionAction().getEnabledDimensions();

        notifyProgressSection("Initializing HSNE hierarchy");

        // Initialize the HSNE algorithm with the given parameters
        _hierarchy.initialize(_core, inputData, enabledDimensions, _hsneSettingsAction.getHsneParameters());

        notifyProgressSection("Computing top-level embedding");

        computeTopLevelEmbedding();
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

    connect(&_tsneAnalysis, &TsneAnalysis::finished, this, [this]() {
        _hsneSettingsAction.getStartAction().setChecked(false);
    });

    connect(&_tsneAnalysis, &TsneAnalysis::embeddingUpdate, this, [this](const TsneData& tsneData) {
        Points& embedding = _core->requestData<Points>(_outputDatasetName);

        embedding.setData(tsneData.getData().data(), tsneData.getNumPoints(), 2);

        _core->notifyDataChanged(_outputDatasetName);
    });
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
    QString subsetName = inputData.createSubset("", false);
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
    _tsneAnalysis.stopComputation();
    _tsneAnalysis.startComputation(tsneParameters, _hierarchy.getTransitionMatrixAtScale(topScaleIndex), numLandmarks, _hierarchy.getNumDimensions());
}

QIcon HsneAnalysisPlugin::getIcon() const
{
    return QIcon(":/images/icon.png");
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
