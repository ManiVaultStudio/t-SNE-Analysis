#pragma once

#include <widgets/SettingsWidget.h>

#include "DimensionSelectionWidget.h"
//#include "HsneSettingsWidget.h"

// Qt header files:
#include <QComboBox>
#include <QPushButton>

using namespace hdps::gui;

enum Algorithm
{
    TSNE, HSNE
};

/**
 * General settings widget containing settings
 * that apply to all algorithms.
 */
class GeneralSettingsWidget : public SettingsWidget
{
    Q_OBJECT

public:
    // Explicitly delete its copy and move member functions.
    GeneralSettingsWidget(const GeneralSettingsWidget&) = delete;
    GeneralSettingsWidget(GeneralSettingsWidget&&) = delete;
    GeneralSettingsWidget& operator=(const GeneralSettingsWidget&) = delete;
    GeneralSettingsWidget& operator=(GeneralSettingsWidget&&) = delete;

    GeneralSettingsWidget();

    const Algorithm& getSelectedAlgorithm();
    QString getCurrentDataItem();
    void addDataItem(const QString name);
    void removeDataItem(const QString name);

    hdps::DimensionSelectionWidget& getDimensionSelectionWidget();

signals:
    void dataSetPicked(QString);
    void startComputation();
    void stopComputation();

public slots:
    void onComputationStopped();

private slots:
    void onAlgorithmPicked(int index);
    void onStartToggled(bool pressed);

private:
    Algorithm _algorithm;

    // UI Settings
    QComboBox* _algorithmPicker;
    QComboBox* _dataOptions;
    hdps::DimensionSelectionWidget _dimensionSelectionWidget;
    SettingsWidget* _hsneSettingsWidget;
    QPushButton* _startButton;
};
