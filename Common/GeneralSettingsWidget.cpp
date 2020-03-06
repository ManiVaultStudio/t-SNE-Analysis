#include "GeneralSettingsWidget.h"

GeneralSettingsWidget::GeneralSettingsWidget() :
    _algorithm(TSNE)
{
    // Set the size of the settings widget
    const auto minimumWidth = 200;
    setMinimumWidth(minimumWidth);
    setMaximumWidth(2 * minimumWidth);

    // Initialize algorithm picker
    _algorithmPicker = new QComboBox();
    _algorithmPicker->addItem("t-SNE");
    _algorithmPicker->addItem("H-SNE");

    connect(_algorithmPicker, SIGNAL(currentIndexChanged(int)), this, SLOT(onAlgorithmPicked(int)));

    // Initialize data options
    _dataOptions = new QComboBox();
    connect(_dataOptions, SIGNAL(currentIndexChanged(QString)), this, SIGNAL(dataSetPicked(QString)));

    // Initialize HSNE settings
    _hsneSettingsWidget = new HsneSettingsWidget();

    // Initialize start button
    _startButton = new QPushButton();
    _startButton->setText("Start Computation");
    _startButton->setFixedSize(QSize(150, 50));
    _startButton->setCheckable(true);
    connect(_startButton, &QPushButton::toggled, this, &GeneralSettingsWidget::onStartToggled);

    // Add all the parts of the settings widget together
    addWidget(_algorithmPicker);
    addWidget(_dataOptions);
    addWidget(&_dimensionSelectionWidget);
    addWidget(_hsneSettingsWidget);
    addWidget(_startButton);
}

const Algorithm& GeneralSettingsWidget::getSelectedAlgorithm()
{
    return _algorithm;
}

QString GeneralSettingsWidget::getCurrentDataItem()
{
    return _dataOptions->currentText();
}

void GeneralSettingsWidget::addDataItem(const QString name)
{
    _dataOptions->addItem(name);
}

void GeneralSettingsWidget::removeDataItem(const QString name)
{
    int index = _dataOptions->findText(name);
    _dataOptions->removeItem(index);
}

hdps::DimensionSelectionWidget& GeneralSettingsWidget::getDimensionSelectionWidget()
{
    return _dimensionSelectionWidget;
}

// SLOTS
void GeneralSettingsWidget::onAlgorithmPicked(int index)
{
    switch (index)
    {
    case 0: _algorithm = TSNE; break;
    case 1: _algorithm = HSNE; break;
    default: _algorithm = TSNE;
    }
}

void GeneralSettingsWidget::onComputationStopped()
{
    _startButton->setText("Start Computation");
    _startButton->setChecked(false);
}

void GeneralSettingsWidget::onStartToggled(bool pressed)
{
    // Do nothing if we have no data set selected
    if (getCurrentDataItem().isEmpty()) {
        return;
    }

    // Check if the tSNE settings are valid before running the computation
    if (_algorithm == TSNE)
    {
        //if (!hasValidSettings()) {
        //    QMessageBox warningBox;
        //    warningBox.setText(tr("Some settings are invalid or missing. Continue with default values?"));
        //    QPushButton *continueButton = warningBox.addButton(tr("Continue"), QMessageBox::ActionRole);
        //    QPushButton *abortButton = warningBox.addButton(QMessageBox::Abort);

        //    warningBox.exec();

        //    if (warningBox.clickedButton() == abortButton) {
        //        return;
        //    }
        //}
    }
    if (_algorithm == HSNE)
    {

    }

    _startButton->setText(pressed ? "Stop Computation" : "Start Computation");
    emit pressed ? startComputation() : stopComputation();
}
