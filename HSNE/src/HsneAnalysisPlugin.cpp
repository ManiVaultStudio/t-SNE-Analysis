#include "HsneAnalysisPlugin.h"

#include "PointData.h"
#include "HsneParameters.h"

#include <QtCore>
#include <QtDebug>

Q_PLUGIN_METADATA(IID "nl.tudelft.HsneAnalysisPlugin")
#include <windows.h>

#include <set>

// =============================================================================
// View
// =============================================================================

using namespace hdps;

HsneAnalysisPlugin::~HsneAnalysisPlugin(void)
{
    
}

void HsneAnalysisPlugin::init()
{
    // Create a new settings widget which allows users to change the parameters given to the HSNE analysis
    _settings = std::make_unique<HsneSettingsWidget>();

    // If a different input dataset is picked in the settings widget update the dimension widget
    connect(_settings.get(), &HsneSettingsWidget::dataSetPicked, this, &HsneAnalysisPlugin::dataSetPicked);
    // If the start computation button is pressed, run the HSNE algorithm
    connect(_settings.get(), &HsneSettingsWidget::startComputation, this, &HsneAnalysisPlugin::startComputation);

    //connect(_settings.get(), &HsneSettingsWidget::stopComputation, this, &HsneAnalysisPlugin::stopComputation);
    //connect(&_tsne, &TsneAnalysis::computationStopped, _settings.get(), &HsneSettingsWidget::onComputationStopped);
    connect(&_tsne, &TsneAnalysis::newEmbedding, this, &HsneAnalysisPlugin::onNewEmbedding);
    //connect(&_tsne, SIGNAL(newEmbedding()), this, SLOT(onNewEmbedding()));
}

void HsneAnalysisPlugin::dataAdded(const QString name)
{
    _settings->addDataItem(name);
}

void HsneAnalysisPlugin::dataChanged(const QString name)
{
    // If we are not looking at the changed dataset, ignore it
    if (name != _settings->getCurrentDataItem()) {
        return;
    }

    // Passes changes to the current dataset to the dimension selection widget
    Points& points = _core->requestData<Points>(name);

    _settings->getDimensionSelectionWidget().dataChanged(points);
}

void HsneAnalysisPlugin::dataRemoved(const QString name)
{
    _settings->removeDataItem(name);
}

void HsneAnalysisPlugin::selectionChanged(const QString dataName)
{
    // Unused in analysis
}

DataTypes HsneAnalysisPlugin::supportedDataTypes() const
{
    DataTypes supportedTypes;
    supportedTypes.append(PointType);
    return supportedTypes;
}

SettingsWidget* const HsneAnalysisPlugin::getSettings()
{
    return _settings.get();
}

// If a different input dataset is picked in the settings widget update the dimension widget
void HsneAnalysisPlugin::dataSetPicked(const QString& name)
{
    Points& points = _core->requestData<Points>(name);

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

    computeEmbedding();
}

void HsneAnalysisPlugin::onNewEmbedding() {
    const TsneData& outputData = _tsne.output();
    Points& embedding = _core->requestData<Points>(_embeddingName);

    embedding.setData(outputData.getData().data(), outputData.getNumPoints(), 2);

    _core->notifyDataChanged(_embeddingName);
}

QString HsneAnalysisPlugin::createEmptyEmbedding(QString name, QString dataType, QString sourceName)
{
    QString embeddingName = _core->createDerivedData(dataType, name, sourceName);
    Points& embedding = _core->requestData<Points>(embeddingName);
    embedding.setData(nullptr, 0, 2);

    auto analyses = embedding.getProperty("Analyses", QVariantList()).toList();
    analyses.push_back(getName());
    embedding.setProperty("Analyses", analyses);

    _core->notifyDataAdded(embeddingName);
    return embeddingName;
}

void HsneAnalysisPlugin::computeEmbedding()
{
    // Create a new data set for the embedding
    _embeddingName = createEmptyEmbedding("Embedding", "Points", _hierarchy.getInputDataName());

    // Should come from some t-SNE settings widget
    _tsne.setIterations(1000);
    _tsne.setPerplexity(30);
    _tsne.setExaggerationIter(250);
    _tsne.setNumTrees(4);
    _tsne.setNumChecks(1024);

    if (_tsne.isRunning())
    {
        // Request interruption of the computation
        _tsne.stopGradientDescent();
        _tsne.exit();

        // Wait until the thread has terminated (max. 3 seconds)
        if (!_tsne.wait(3000))
        {
            qDebug() << "tSNE computation thread did not close in time, terminating...";
            _tsne.terminate();
            _tsne.wait();
        }
        qDebug() << "tSNE computation stopped.";
    }
    // Initialize tsne, compute data to be embedded, start computation?
    _tsne.initWithProbDist(_hierarchy.getNumPoints(), _hierarchy.getNumDimensions(), _hierarchy.getTransitionMatrixAtScale(_hierarchy.getCurrentScale())); // FIXME
    // Embed data
    _tsne.start();
}

void HsneAnalysisPlugin::onDrillIn()
{
    //_hsne.drillIn("Temp");
}

void HsneAnalysisPlugin::drillIn(QString embeddingName)
{
    Points& embedding = _core->requestData<Points>(embeddingName);
    Points& source = hdps::DataSet::getSourceData<Points>(embedding);
    //QString subset = source.createSubset();

    Points& selection = static_cast<Points&>(embedding.getSelection());
    //_hsne->scale(scale)._area_of_influence[selection.indices];

    std::map<uint32_t, float> neighbors;
    _hierarchy.getInfluencedLandmarksInPreviousScale(selection.indices, neighbors);

    std::vector<uint32_t> nextLevelIdxs;
    nextLevelIdxs.clear();
    for (auto n : neighbors) {
        if (n.second > 0.5) //QUICKPAPER
        {
            nextLevelIdxs.push_back(n.first);
        }
    }
    std::cout << "#landmarks at previous scale: " << neighbors.size() << std::endl;
    std::cout << "Drilling in" << std::endl;
}


// =============================================================================
// Factory
// =============================================================================

AnalysisPlugin* HsneAnalysisPluginFactory::produce()
{
    return new HsneAnalysisPlugin();
}
