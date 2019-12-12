#include "TsneAnalysisPlugin.h"

#include "PointData.h"

#include <QtCore>
#include <QtDebug>

Q_PLUGIN_METADATA(IID "nl.tudelft.TsneAnalysisPlugin")
#include <windows.h>
// =============================================================================
// View
// =============================================================================

using namespace hdps;

TsneAnalysisPlugin::~TsneAnalysisPlugin(void)
{
    stopComputation();
}

void TsneAnalysisPlugin::init()
{
    _settings = std::make_unique<TsneSettingsWidget>();

    connect(_settings.get(), &TsneSettingsWidget::dataSetPicked, this, &TsneAnalysisPlugin::dataSetPicked);
    connect(_settings.get(), &TsneSettingsWidget::startComputation, this, &TsneAnalysisPlugin::startComputation);
    connect(_settings.get(), &TsneSettingsWidget::stopComputation, this, &TsneAnalysisPlugin::stopComputation);
    connect(&_tsne, &TsneAnalysis::computationStopped, _settings.get(), &TsneSettingsWidget::computationStopped);
    connect(&_tsne, SIGNAL(newEmbedding()), this, SLOT(onNewEmbedding()));
}

void TsneAnalysisPlugin::dataAdded(const QString name)
{
    _settings->dataOptions.addItem(name);
}

void TsneAnalysisPlugin::dataChanged(const QString name)
{
    if (name != _settings->currentData()) {
        return;
    }

    Points& points = _core->requestData<Points>(name);

    _settings->dataChanged(points);
}

void TsneAnalysisPlugin::dataRemoved(const QString name)
{
    
}

void TsneAnalysisPlugin::selectionChanged(const QString dataName)
{

}


DataTypes TsneAnalysisPlugin::supportedDataTypes() const
{
    DataTypes supportedTypes;
    supportedTypes.append(PointType);
    return supportedTypes;
}

SettingsWidget* const TsneAnalysisPlugin::getSettings()
{
    return _settings.get();
}

void TsneAnalysisPlugin::dataSetPicked(const QString& name)
{
    Points& points = _core->requestData<Points>(name);

    _settings->dataChanged(points);
}

void TsneAnalysisPlugin::startComputation()
{
    initializeTsne();

    // Run the computation
    QString setName = _settings->dataOptions.currentText();
    const Points& points = _core->requestData<Points>(setName);

    // Create list of data from the enabled dimensions
    std::vector<float> data;
    
    std::vector<bool> enabledDimensions = _settings->getEnabledDimensions();
    for (int i = 0; i < points.getNumPoints(); i++)
    {
        for (int j = 0; j < points.getNumDimensions(); j++)
        {
            if (enabledDimensions[j])
                data.push_back(points[i * points.getNumDimensions() + j]);
        }
    }
    unsigned int numDimensions = count_if(enabledDimensions.begin(), enabledDimensions.end(), [](bool b) { return b; } );

    _embeddingName = _core->createDerivedData("Points", "Embedding", points.getName());
    Points& embedding = _core->requestData<Points>(_embeddingName);
    
    embedding.setData(nullptr, 0, 2);
    _core->notifyDataAdded(_embeddingName);

    // Compute t-SNE with the given data
    _tsne.initTSNE(data, numDimensions);

    _tsne.start();
}

void TsneAnalysisPlugin::onNewEmbedding() {
    const TsneData& outputData = _tsne.output();
    Points& embedding = _core->requestData<Points>(_embeddingName);
    
    embedding.setData(outputData.getData().data(), outputData.getNumPoints(), 2);

    _core->notifyDataChanged(_embeddingName);
}

void TsneAnalysisPlugin::initializeTsne() {
    // Initialize the tSNE computation with the settings from the settings widget
    _tsne.setIterations(_settings->numIterations.text().toInt());
    _tsne.setPerplexity(_settings->perplexity.text().toInt());
    _tsne.setExaggerationIter(_settings->exaggeration.text().toInt());
    _tsne.setNumTrees(_settings->numTrees.text().toInt());
    _tsne.setNumChecks(_settings->numChecks.text().toInt());
}

void TsneAnalysisPlugin::stopComputation() {
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
}

// =============================================================================
// Factory
// =============================================================================

AnalysisPlugin* TsneAnalysisPluginFactory::produce()
{
    return new TsneAnalysisPlugin();
}
