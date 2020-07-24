#include "TsneSettingsWidget.h"


#include "DimensionSelectionWidget.h"
#include "TsneAnalysisPlugin.h"

// Qt header files:
#include <QDebug>
#include <QFileDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QScrollArea>
#include <QVBoxLayout>



TsneSettingsWidget::TsneSettingsWidget(TsneAnalysisPlugin& analysisPlugin)
:
_analysisPlugin(analysisPlugin)
{
    const auto minimumWidth = 200;
    setMinimumWidth(minimumWidth);
    setMaximumWidth(2 * minimumWidth);

    knnOptions.addItem("FLANN");
    knnOptions.addItem("HNSW");
    knnOptions.addItem("ANNOY");

    distanceMetric.addItem("Euclidean");
    distanceMetric.addItem("Cosine");
    distanceMetric.addItem("Inner Product");
    distanceMetric.addItem("Manhattan");
    distanceMetric.addItem("Hamming");
    distanceMetric.addItem("Dot");

    connect(&dataOptions,   SIGNAL(currentIndexChanged(QString)), this, SIGNAL(dataSetPicked(QString)));
    connect(&knnOptions,    SIGNAL(currentIndexChanged(int)), this, SIGNAL(knnAlgorithmPicked(int)));
    connect(&distanceMetric,SIGNAL(currentIndexChanged(int)), this, SIGNAL(distanceMetricPicked(int)));
    connect(&startButton,   &QPushButton::toggled, this, &TsneSettingsWidget::onStartToggled);
    //////////////////////////////

    // Create group boxes for grouping together various settings
    QGroupBox* settingsBox = new QGroupBox("Basic settings");
    QGroupBox* advancedSettingsBox = new QGroupBox("Advanced Settings");

    advancedSettingsBox->setCheckable(true);
    advancedSettingsBox->setChecked(false);

    // Build the labels for all the options
    QLabel* iterationLabel = new QLabel("Iteration Count");
    QLabel* perplexityLabel = new QLabel("Perplexity");
    QLabel* knnAlgorithmLabel = new QLabel("KNN Algorithm");
    QLabel* distanceMetricLabel = new QLabel("Distance Metric");
    QLabel* exaggerationLabel = new QLabel("Exaggeration");
    QLabel* expDecayLabel = new QLabel("Exponential Decay");
    QLabel* numTreesLabel = new QLabel("Number of Trees");
    QLabel* numChecksLabel = new QLabel("Number of Checks");

    // Set option default values
    numIterations.setFixedWidth(50);
    perplexity.setFixedWidth(50);
    exaggeration.setFixedWidth(50);
    expDecay.setFixedWidth(50);
    numTrees.setFixedWidth(50);
    numChecks.setFixedWidth(50);

    numIterations.setValidator(new QIntValidator(1, 10000, this));
    perplexity.setValidator(new QIntValidator(2, 50, this));
    exaggeration.setValidator(new QIntValidator(1, 10000, this));
    expDecay.setValidator(new QIntValidator(1, 10000, this));
    numTrees.setValidator(new QIntValidator(1, 10000, this));
    numChecks.setValidator(new QIntValidator(1, 10000, this));

    numIterations.setText("1000");
    perplexity.setText("30");
    exaggeration.setText("250");
    expDecay.setText("70");
    numTrees.setText("4");
    numChecks.setText("1024");

    // Add options to their appropriate group box
    auto* const settingsLayout = new QVBoxLayout();
    settingsLayout->addWidget(knnAlgorithmLabel);
    settingsLayout->addWidget(&knnOptions);
    settingsLayout->addWidget(distanceMetricLabel);
    settingsLayout->addWidget(&distanceMetric);
    settingsLayout->addWidget(iterationLabel);
    settingsLayout->addWidget(&numIterations);
    settingsLayout->addWidget(perplexityLabel);
    settingsLayout->addWidget(&perplexity);
    settingsBox->setLayout(settingsLayout);

    auto* const advancedSettingsLayout = new QGridLayout();
    advancedSettingsLayout->addWidget(exaggerationLabel, 0, 0);
    advancedSettingsLayout->addWidget(&exaggeration, 1, 0);
    advancedSettingsLayout->addWidget(expDecayLabel, 0, 1);
    advancedSettingsLayout->addWidget(&expDecay, 1, 1);
    advancedSettingsLayout->addWidget(numTreesLabel, 2, 0);
    advancedSettingsLayout->addWidget(&numTrees, 3, 0);
    advancedSettingsLayout->addWidget(numChecksLabel, 2, 1);
    advancedSettingsLayout->addWidget(&numChecks, 3, 1);
    advancedSettingsBox->setLayout(advancedSettingsLayout);
    /////////////////////////////////////////////////////////////////////

    // Initialize start button
    _startButton = new QPushButton();
    _startButton->setText("Compute Embedding");
    _startButton->setFixedSize(QSize(150, 50));
    _startButton->setCheckable(true);
    connect(_startButton, &QPushButton::toggled, this, &TsneSettingsWidget::onStartToggled);

    // Add all the parts of the settings widget together
    addWidget(_dataOptions);
    addWidget(&_dimensionSelectionWidget);
    addWidget(settingsBox);
    addWidget(advancedSettingsBox);
    addWidget(_startButton);
}

QString TsneSettingsWidget::getCurrentDataItem()
{
    return _dataOptions->currentText();
}

void TsneSettingsWidget::addDataItem(const QString name)
{
    _dataOptions->addItem(name);
}

void TsneSettingsWidget::removeDataItem(const QString name)
{
    int index = _dataOptions->findText(name);
    _dataOptions->removeItem(index);
}

void TsneSettingsWidget::onComputationStopped()
{
    _startButton->setText("Start Computation");
    _startButton->setChecked(false);
}

void TsneSettingsWidget::onStartToggled(bool pressed)
{
    // Do nothing if we have no data set selected
    if (getCurrentDataItem().isEmpty()) {
        return;
    }

    // Check if the tSNE settings are valid before running the computation
    if (!hasValidSettings()) {
        QMessageBox warningBox;
        warningBox.setText(tr("Some settings are invalid or missing. Continue with default values?"));
        QPushButton *continueButton = warningBox.addButton(tr("Continue"), QMessageBox::ActionRole);
        QPushButton *abortButton = warningBox.addButton(QMessageBox::Abort);

        warningBox.exec();

        if (warningBox.clickedButton() == abortButton) {
            return;
        }
    }

    _startButton->setText(pressed ? "Stop Computation" : "Start Computation");
    emit pressed ? startComputation() : stopComputation();
}

void TsneSettingsWidget::dataChanged(const Points& points)
{
    _dimensionSelectionWidget.dataChanged(points);
}

std::vector<bool> TsneSettingsWidget::getEnabledDimensions()
{
    return _dimensionSelectionWidget.getEnabledDimensions();
}

// Check if all input values are valid
bool TsneSettingsWidget::hasValidSettings()
{
    if (!numIterations.hasAcceptableInput())
        return false;
    if (!perplexity.hasAcceptableInput())
        return false;
    if (!exaggeration.hasAcceptableInput())
        return false;
    if (!expDecay.hasAcceptableInput())
        return false;
    if (!numTrees.hasAcceptableInput())
        return false;
    if (!numChecks.hasAcceptableInput())
        return false;

    return true;
}

void TsneSettingsWidget::checkInputStyle(QLineEdit& input)
{
    if (input.hasAcceptableInput())
    {
        input.setStyleSheet("");
    }
    else
    {
        input.setStyleSheet("border: 1px solid red");
    }
}

hdps::DimensionSelectionWidget& TsneSettingsWidget::getDimensionSelectionWidget()
{
    return _dimensionSelectionWidget;
}


// SLOTS
void TsneSettingsWidget::onStartToggled(bool pressed)
{
    // Do nothing if we have no data set selected
    if (dataOptions.currentText().isEmpty()) {
        return;
    }

    // Check if the tSNE settings are valid before running the computation
    if (!hasValidSettings()) {
        QMessageBox warningBox;
        warningBox.setText(tr("Some settings are invalid or missing. Continue with default values?"));
        QPushButton *continueButton = warningBox.addButton(tr("Continue"), QMessageBox::ActionRole);
        QPushButton *abortButton = warningBox.addButton(QMessageBox::Abort);

        warningBox.exec();

        if (warningBox.clickedButton() == abortButton) {
            return;
        }
    }
    startButton.setText(pressed ? "Stop Computation" : "Start Computation");
    pressed ? _analysisPlugin.startComputation() : _analysisPlugin.stopComputation();;
}

void TsneSettingsWidget::numIterationsChanged(const QString &)
{
    checkInputStyle(numIterations);
}

void TsneSettingsWidget::perplexityChanged(const QString &)
{
    checkInputStyle(perplexity);
}

void TsneSettingsWidget::exaggerationChanged(const QString &)
{
    checkInputStyle(exaggeration);
}

void TsneSettingsWidget::expDecayChanged(const QString &)
{
    checkInputStyle(expDecay);
}

void TsneSettingsWidget::numTreesChanged(const QString &)
{
    checkInputStyle(numTrees);
}

void TsneSettingsWidget::numChecksChanged(const QString &)
{
    checkInputStyle(numChecks);
}

void TsneSettingsWidget::thetaChanged(const QString& )
{
    checkInputStyle(theta);
}
