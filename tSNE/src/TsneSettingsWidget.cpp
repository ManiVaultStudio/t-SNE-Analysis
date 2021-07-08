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



TsneSettingsWidget::TsneSettingsWidget(TsneAnalysisPlugin& analysisPlugin) :
    SettingsWidget(),
    _analysisPlugin(analysisPlugin),
    _tsneOptions(TsneParameters())
{
    const auto guiName = analysisPlugin.getGuiName();

    setObjectName(guiName);

    setIcon(hdps::Application::getIconFont("FontAwesome").getIcon("th"));
    setTitle(guiName);
    setSubtitle("");

    connect(_tsneOptions.knnOptions,    SIGNAL(currentIndexChanged(int)), this, SIGNAL(knnAlgorithmPicked(int)));
    connect(_tsneOptions.distanceMetric,SIGNAL(currentIndexChanged(int)), this, SIGNAL(distanceMetricPicked(int)));

    connect(_tsneOptions.numIterations, SIGNAL(textChanged(QString)), SLOT(numIterationsChanged(QString)));
    connect(_tsneOptions.perplexity, SIGNAL(textChanged(QString)), SLOT(perplexityChanged(QString)));
    connect(_tsneOptions.numTrees, SIGNAL(textChanged(QString)), SLOT(numTreesChanged(QString)));
    connect(_tsneOptions.numChecks, SIGNAL(textChanged(QString)), SLOT(numChecksChanged(QString)));
    connect(_tsneOptions.exaggeration, SIGNAL(textChanged(QString)), SLOT(exaggerationChanged(QString)));
    connect(_tsneOptions.expDecay, SIGNAL(textChanged(QString)), SLOT(expDecayChanged(QString)));

    // Initialize data options
    _dataOptions = new QComboBox();
    connect(_dataOptions, SIGNAL(currentIndexChanged(QString)), this, SIGNAL(dataSetPicked(QString)));
    connect(_dataOptions, SIGNAL(currentIndexChanged(QString)), this, SLOT(setEmbeddingName(QString)));

    // Initialize start button
    _startButton = new QPushButton();
    _startButton->setText("Compute Embedding");
    _startButton->setFixedSize(QSize(150, 50));
    _startButton->setCheckable(true);
    connect(_startButton, &QPushButton::toggled, this, &TsneSettingsWidget::onStartToggled);

    // Create group boxes for grouping together various settings
    QGroupBox* settingsBox = new QGroupBox("Basic settings");
    QGroupBox* advancedSettingsBox = new QGroupBox("Advanced Settings");
    QGroupBox* computeBox = new QGroupBox();

    advancedSettingsBox->setCheckable(true);
    advancedSettingsBox->setChecked(false);

    // Set option default values
    _tsneOptions.numIterations->setFixedWidth(50);
    _tsneOptions.perplexity->setFixedWidth(50);
    _tsneOptions.numTrees->setFixedWidth(50);
    _tsneOptions.numChecks->setFixedWidth(50);
    _tsneOptions.exaggeration->setFixedWidth(50);
    _tsneOptions.expDecay->setFixedWidth(50);

    _tsneOptions.numIterations->setValidator(new QIntValidator(1, 10000, this));
    _tsneOptions.perplexity->setValidator(new QIntValidator(2, 50, this));
    _tsneOptions.numTrees->setValidator(new QIntValidator(1, 10000, this));
    _tsneOptions.numChecks->setValidator(new QIntValidator(1, 10000, this));
    _tsneOptions.exaggeration->setValidator(new QIntValidator(1, 10000, this));
    _tsneOptions.expDecay->setValidator(new QIntValidator(1, 10000, this));

    // Add options to their appropriate group box
    auto* const settingsLayout = new QVBoxLayout();
    settingsLayout->addWidget(_tsneOptions.knnAlgorithmLabel);
    settingsLayout->addWidget(_tsneOptions.knnOptions);
    settingsLayout->addWidget(_tsneOptions.distanceMetricLabel);
    settingsLayout->addWidget(_tsneOptions.distanceMetric);
    settingsLayout->addWidget(_tsneOptions.iterationLabel);
    settingsLayout->addWidget(_tsneOptions.numIterations);
    settingsLayout->addWidget(_tsneOptions.perplexityLabel);
    settingsLayout->addWidget(_tsneOptions.perplexity);
    settingsBox->setLayout(settingsLayout);

    auto* const advancedSettingsLayout = new QGridLayout();
    advancedSettingsLayout->addWidget(_tsneOptions.exaggerationLabel, 0, 0);
    advancedSettingsLayout->addWidget(_tsneOptions.exaggeration, 1, 0);
    advancedSettingsLayout->addWidget(_tsneOptions.expDecayLabel, 0, 1);
    advancedSettingsLayout->addWidget(_tsneOptions.expDecay, 1, 1);
    advancedSettingsLayout->addWidget(_tsneOptions.numTreesLabel, 2, 0);
    advancedSettingsLayout->addWidget(_tsneOptions.numTrees, 3, 0);
    advancedSettingsLayout->addWidget(_tsneOptions.numChecksLabel, 2, 1);
    advancedSettingsLayout->addWidget(_tsneOptions.numChecks, 3, 1);
    advancedSettingsBox->setLayout(advancedSettingsLayout);

    auto* const computeLayout = new QGridLayout();
    QLabel* embeddingNameLabel = new QLabel("Embedding Name");
    embeddingNameLine = new QLineEdit("Embedding");
    computeLayout->addWidget(embeddingNameLabel, 0, 0);
    computeLayout->addWidget(embeddingNameLine, 1, 0, Qt::AlignTop);
    computeLayout->addWidget(_startButton, 0, 1, 2, 1, Qt::AlignCenter);
    computeBox->setLayout(computeLayout);

    // Add all the parts of the settings widget together
    addWidget(_dataOptions);
    addWidget(&_dimensionSelectionWidget);
    addWidget(settingsBox);
    addWidget(advancedSettingsBox);
    addWidget(computeBox);
}

void TsneSettingsWidget::setEmbeddingName(QString embName)
{
    embeddingNameLine->setText(embName + "_tsne_emb");
}

QString TsneSettingsWidget::getEmbeddingName()
{
    return embeddingNameLine->text();
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

	setSubtitle("");
	setIcon(hdps::Application::getIconFont("FontAwesome").getIcon("th"));
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
    pressed ? _analysisPlugin.startComputation() : _analysisPlugin.stopComputation();
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
    if (!_tsneOptions.numIterations->hasAcceptableInput())
        return false;
    if (!_tsneOptions.perplexity->hasAcceptableInput())
        return false;
    if (!_tsneOptions.numTrees->hasAcceptableInput())
        return false;
    if (!_tsneOptions.numChecks->hasAcceptableInput())
        return false;
    if (!_tsneOptions.exaggeration->hasAcceptableInput())
        return false;
    if (!_tsneOptions.expDecay->hasAcceptableInput())
        return false;

    return true;
}

void TsneSettingsWidget::checkInputStyle(QLineEdit* input)
{
    if (input->hasAcceptableInput())
    {
        input->setStyleSheet("");
    }
    else
    {
        input->setStyleSheet("border: 1px solid red");
    }
}

hdps::DimensionSelectionWidget& TsneSettingsWidget::getDimensionSelectionWidget()
{
    return _dimensionSelectionWidget;
}

TsneParameters TsneSettingsWidget::getTsneParameters() const
{
    return _tsneOptions.getTsneParameters();
}

void TsneSettingsWidget::numIterationsChanged(const QString &)
{
    checkInputStyle(_tsneOptions.numIterations);
}

void TsneSettingsWidget::perplexityChanged(const QString &)
{
    checkInputStyle(_tsneOptions.perplexity);
}

void TsneSettingsWidget::exaggerationChanged(const QString &)
{
    checkInputStyle(_tsneOptions.exaggeration);
}

void TsneSettingsWidget::expDecayChanged(const QString &)
{
    checkInputStyle(_tsneOptions.expDecay);
}

void TsneSettingsWidget::numTreesChanged(const QString &)
{
    checkInputStyle(_tsneOptions.numTrees);
}

void TsneSettingsWidget::numChecksChanged(const QString &)
{
    checkInputStyle(_tsneOptions.numChecks);
}
