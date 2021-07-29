#include "HsneSettingsWidget.h"

#include "HsneAnalysisPlugin.h"

#include <QGroupBox>

HsneSettingsWidget::HsneSettingsWidget(HsneAnalysisPlugin& analysisPlugin) :
    _analysisPlugin(analysisPlugin),
    _hsneOptions(HsneParameters()),
    _tsneOptions(TsneParameters())
{
    // Set the size of the settings widget
    const auto minimumWidth = 200;
    setMinimumWidth(minimumWidth);
    setMaximumWidth(2 * minimumWidth);

    // Initialize data options
    _dataOptions = new QComboBox();
    connect(_dataOptions, SIGNAL(currentIndexChanged(QString)), this, SIGNAL(dataSetPicked(QString)));
    connect(_dataOptions, SIGNAL(currentIndexChanged(QString)), this, SLOT(setEmbeddingName(QString)));

    // Initialize start button
    _startButton = new QPushButton();
    _startButton->setText("Start Computation");
    _startButton->setFixedSize(QSize(150, 50));
    _startButton->setCheckable(true);
    connect(_startButton, &QPushButton::toggled, this, &HsneSettingsWidget::onStartToggled);

    _hsneOptions.knnOptions->addItem("FLANN");
    _hsneOptions.knnOptions->addItem("HNSW");
    _hsneOptions.knnOptions->addItem("ANNOY");

    // Create group boxes for grouping together various settings
    QGroupBox* hsneSettingsBox = new QGroupBox("HSNE settings");
    QGroupBox* tsneSettingsBox = new QGroupBox("TSNE settings");
    QGroupBox* computeBox = new QGroupBox();

    // Set HSNE-specific settings layout
    QGridLayout* coreSettingsLayout = new QGridLayout();
    //coreSettingsLayout->addWidget(_options.knnOptions, 0, 1);
    coreSettingsLayout->addWidget(_hsneOptions.seedLabel, 1, 0);
    coreSettingsLayout->addWidget(_hsneOptions.useMonteCarloSamplingLabel, 2, 0);
    coreSettingsLayout->addWidget(_hsneOptions.numWalksForLandmarkSelectionLabel, 3, 0);
    coreSettingsLayout->addWidget(_hsneOptions.numWalksForLandmarkSelectionThresholdLabel, 4, 0);
    coreSettingsLayout->addWidget(_hsneOptions.randomWalkLengthLabel, 5, 0);
    coreSettingsLayout->addWidget(_hsneOptions.numWalksForAreaOfInfluenceLabel, 6, 0);
    coreSettingsLayout->addWidget(_hsneOptions.minWalksRequiredLabel, 7, 0);
    coreSettingsLayout->addWidget(_hsneOptions.numChecksAknnLabel, 8, 0);
    coreSettingsLayout->addWidget(_hsneOptions.useOutOfCoreComputationLabel, 9, 0);

    coreSettingsLayout->addWidget(_hsneOptions.knnOptions, 0, 1);
    coreSettingsLayout->addWidget(_hsneOptions.seed, 1, 1);
    coreSettingsLayout->addWidget(_hsneOptions.useMonteCarloSampling, 2, 1);
    coreSettingsLayout->addWidget(_hsneOptions.numWalksForLandmarkSelection, 3, 1);
    coreSettingsLayout->addWidget(_hsneOptions.numWalksForLandmarkSelectionThreshold, 4, 1);
    coreSettingsLayout->addWidget(_hsneOptions.randomWalkLength, 5, 1);
    coreSettingsLayout->addWidget(_hsneOptions.numWalksForAreaOfInfluence, 6, 1);
    coreSettingsLayout->addWidget(_hsneOptions.minWalksRequired, 7, 1);
    coreSettingsLayout->addWidget(_hsneOptions.numChecksAknn, 8, 1);
    coreSettingsLayout->addWidget(_hsneOptions.useOutOfCoreComputation, 9, 1);

    // Set TSNE specific settings layout
    QGridLayout* coreTsneSettingsLayout = new QGridLayout();
    coreTsneSettingsLayout->addWidget(_tsneOptions.numIterationsLabel, 0, 0);
    coreTsneSettingsLayout->addWidget(_tsneOptions.perplexityLabel, 1, 0);
    coreTsneSettingsLayout->addWidget(_tsneOptions.numTreesLabel, 2, 0);
    coreTsneSettingsLayout->addWidget(_tsneOptions.numChecksLabel, 3, 0);
    coreTsneSettingsLayout->addWidget(_tsneOptions.exaggerationLabel, 4, 0);
    coreTsneSettingsLayout->addWidget(_tsneOptions.expDecayLabel, 5, 0);
    coreTsneSettingsLayout->addWidget(_tsneOptions.embeddingNameLabel, 6, 0);

    coreTsneSettingsLayout->addWidget(_tsneOptions.numIterations, 0, 1);
    coreTsneSettingsLayout->addWidget(_tsneOptions.perplexity, 1, 1);
    coreTsneSettingsLayout->addWidget(_tsneOptions.numTrees, 2, 1);
    coreTsneSettingsLayout->addWidget(_tsneOptions.numChecks, 3, 1);
    coreTsneSettingsLayout->addWidget(_tsneOptions.exaggeration, 4, 1);
    coreTsneSettingsLayout->addWidget(_tsneOptions.expDecay, 5, 1);
    coreTsneSettingsLayout->addWidget(_tsneOptions.embeddingName, 6, 1);

    auto* const computeLayout = new QGridLayout();
    QLabel* embeddingNameLabel = new QLabel("Embedding Name");
    computeLayout->addWidget(embeddingNameLabel, 0, 0);
    computeLayout->addWidget(&_embeddingNameLine, 1, 0, Qt::AlignTop);
    computeLayout->addWidget(_startButton, 0, 1, 2, 1, Qt::AlignCenter);

    // Add options to their appropriate group box
    hsneSettingsBox->setLayout(coreSettingsLayout);
    tsneSettingsBox->setLayout(coreTsneSettingsLayout);
    computeBox->setLayout(computeLayout);

    // Add all the parts of the settings widget together
    //addWidget(_dataOptions);
    //addWidget(&_dimensionSelectionWidget);
    //addWidget(hsneSettingsBox);
    //addWidget(tsneSettingsBox);
    //addWidget(computeBox);
}

void HsneSettingsWidget::setEmbeddingName(QString embName)
{
    _embeddingNameLine.setText(embName + "_hsne_emb");
}

QString HsneSettingsWidget::getEmbeddingName()
{
    return _embeddingNameLine.text();
}

QString HsneSettingsWidget::getCurrentDataItem()
{
    return _dataOptions->currentText();
}

void HsneSettingsWidget::addDataItem(const QString name)
{
    _dataOptions->addItem(name);
}

void HsneSettingsWidget::removeDataItem(const QString name)
{
    int index = _dataOptions->findText(name);
    _dataOptions->removeItem(index);
}

hdps::DimensionSelectionWidget& HsneSettingsWidget::getDimensionSelectionWidget()
{
    return _dimensionSelectionWidget;
}

HsneParameters HsneSettingsWidget::getHsneParameters() const
{
    HsneParameters parameters;
    // TODO Check if parameters are valid or not, this should already be done in onStartToggled() but need to check if everything is okay
    // here too. Check for exceptions etc.
    int index = _hsneOptions.knnOptions->currentIndex();
    switch (index)
    {
    case 0: parameters.setKnnLibrary(hdi::dr::KNN_FLANN); break;
    case 1: parameters.setKnnLibrary(hdi::dr::KNN_HNSW); break;
    case 2: parameters.setKnnLibrary(hdi::dr::KNN_ANNOY); break;
    }
    parameters.setSeed(_hsneOptions.seed->text().toInt());
    parameters.useMonteCarloSampling(_hsneOptions.useMonteCarloSampling->checkState() == Qt::CheckState::Checked);
    parameters.useOutOfCoreComputation(_hsneOptions.useOutOfCoreComputation->checkState() == Qt::CheckState::Checked);
    parameters.setNumWalksForLandmarkSelection(_hsneOptions.numWalksForLandmarkSelection->text().toInt());
    parameters.setNumWalksForLandmarkSelectionThreshold(_hsneOptions.numWalksForLandmarkSelectionThreshold->text().toFloat());
    parameters.setRandomWalkLength(_hsneOptions.randomWalkLength->text().toInt());
    parameters.setNumWalksForAreaOfInfluence(_hsneOptions.numWalksForAreaOfInfluence->text().toInt());
    parameters.setMinWalksRequired(_hsneOptions.minWalksRequired->text().toInt());
    parameters.setNumChecksAKNN(_hsneOptions.numChecksAknn->text().toInt());

    return parameters;
}

TsneParameters HsneSettingsWidget::getTsneParameters() const
{
    TsneParameters parameters;

    parameters.setNumIterations(_tsneOptions.numIterations->text().toInt());
    parameters.setPerplexity(_tsneOptions.perplexity->text().toInt());
    parameters.setNumTrees(_tsneOptions.numTrees->text().toInt());
    parameters.setNumChecks(_tsneOptions.numChecks->text().toInt());
    parameters.setExaggerationIter(_tsneOptions.exaggeration->text().toInt());
    parameters.setExponentialDecayIter(_tsneOptions.expDecay->text().toInt());

    return parameters;
}

void HsneSettingsWidget::onComputationStopped()
{
    _startButton->setText("Start Computation");
    _startButton->setChecked(false);
}

void HsneSettingsWidget::onStartToggled(bool pressed)
{
    // Do nothing if we have no data set selected
    if (getCurrentDataItem().isEmpty()) {
        return;
    }

    // TODO Check valid settings

    _startButton->setText(pressed ? "Stop Computation" : "Start Computation");
    emit pressed ? startComputation() : stopComputation();
}
