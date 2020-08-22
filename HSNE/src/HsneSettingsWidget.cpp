#include "HsneSettingsWidget.h"

#include "HsneAnalysisPlugin.h"

#include <QGroupBox>

HsneSettingsWidget::HsneSettingsWidget(HsneAnalysisPlugin& analysisPlugin) :
    _analysisPlugin(analysisPlugin),
    _options(HsneParameters())
{
    // Set the size of the settings widget
    const auto minimumWidth = 200;
    setMinimumWidth(minimumWidth);
    setMaximumWidth(2 * minimumWidth);

    // Initialize data options
    _dataOptions = new QComboBox();
    connect(_dataOptions, SIGNAL(currentIndexChanged(QString)), this, SIGNAL(dataSetPicked(QString)));

    // Create group boxes for grouping together various settings
    QGroupBox* settingsBox = new QGroupBox("Basic settings");

    // Set HSNE-specific settings layout
    QGridLayout* coreSettingsLayout = new QGridLayout();
    coreSettingsLayout->addWidget(_options.seedLabel, 0, 0);
    coreSettingsLayout->addWidget(_options.useMonteCarloSamplingLabel, 1, 0);
    coreSettingsLayout->addWidget(_options.numWalksForLandmarkSelectionLabel, 2, 0);
    coreSettingsLayout->addWidget(_options.numWalksForLandmarkSelectionThresholdLabel, 3, 0);
    coreSettingsLayout->addWidget(_options.randomWalkLengthLabel, 4, 0);
    coreSettingsLayout->addWidget(_options.numWalksForAreaOfInfluenceLabel, 5, 0);
    coreSettingsLayout->addWidget(_options.minWalksRequiredLabel, 6, 0);
    coreSettingsLayout->addWidget(_options.numChecksAknnLabel, 7, 0);
    coreSettingsLayout->addWidget(_options.useOutOfCoreComputationLabel, 8, 0);

    coreSettingsLayout->addWidget(_options.seed, 0, 1);
    coreSettingsLayout->addWidget(_options.useMonteCarloSampling, 1, 1);
    coreSettingsLayout->addWidget(_options.numWalksForLandmarkSelection, 2, 1);
    coreSettingsLayout->addWidget(_options.numWalksForLandmarkSelectionThreshold, 3, 1);
    coreSettingsLayout->addWidget(_options.randomWalkLength, 4, 1);
    coreSettingsLayout->addWidget(_options.numWalksForAreaOfInfluence, 5, 1);
    coreSettingsLayout->addWidget(_options.minWalksRequired, 6, 1);
    coreSettingsLayout->addWidget(_options.numChecksAknn, 7, 1);
    coreSettingsLayout->addWidget(_options.useOutOfCoreComputation, 8, 1);

    // Add options to their appropriate group box
    settingsBox->setLayout(coreSettingsLayout);

    // Initialize start button
    _startButton = new QPushButton();
    _startButton->setText("Start Computation");
    _startButton->setFixedSize(QSize(150, 50));
    _startButton->setCheckable(true);
    connect(_startButton, &QPushButton::toggled, this, &HsneSettingsWidget::onStartToggled);


    // Add all the parts of the settings widget together
    addWidget(_dataOptions);
    addWidget(&_dimensionSelectionWidget);
    addWidget(settingsBox);
    addWidget(_startButton);
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
    parameters.setSeed(_options.seed->text().toInt());
    parameters.useMonteCarloSampling(_options.useMonteCarloSampling->checkState() == Qt::CheckState::Checked);
    parameters.useOutOfCoreComputation(_options.useOutOfCoreComputation->checkState() == Qt::CheckState::Checked);
    parameters.setNumWalksForLandmarkSelection(_options.numWalksForLandmarkSelection->text().toInt());
    parameters.setNumWalksForLandmarkSelectionThreshold(_options.numWalksForLandmarkSelectionThreshold->text().toFloat());
    parameters.setRandomWalkLength(_options.randomWalkLength->text().toInt());
    parameters.setNumWalksForAreaOfInfluence(_options.numWalksForAreaOfInfluence->text().toInt());
    parameters.setMinWalksRequired(_options.minWalksRequired->text().toInt());
    parameters.setNumChecksAKNN(_options.numChecksAknn->text().toInt());

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
