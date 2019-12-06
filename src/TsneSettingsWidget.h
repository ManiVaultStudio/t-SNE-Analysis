#pragma once

#include <widgets/SettingsWidget.h>

#include "PointData.h"

#include "DimensionSelectionWidget.h"

// Qt header files:
#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QLineEdit>
#include <QObject>
#include <QPushButton>
#include <QString>

#include <memory> // For unique_ptr
#include <vector>

using namespace hdps::gui;

class TsneAnalysisPlugin;

/**
 * Main settings widget
 */
class TsneSettingsWidget : public SettingsWidget
{
    Q_OBJECT

public:
    // Explicitly delete its copy and move member functions.
    TsneSettingsWidget(const TsneSettingsWidget&) = delete;
    TsneSettingsWidget(TsneSettingsWidget&&) = delete;
    TsneSettingsWidget& operator=(const TsneSettingsWidget&) = delete;
    TsneSettingsWidget& operator=(TsneSettingsWidget&&) = delete;

    TsneSettingsWidget();

    std::vector<bool> getEnabledDimensions();
    bool hasValidSettings();

    QString currentData();
    void dataChanged(const Points& points);
private:
    void checkInputStyle(QLineEdit& input);

signals:
    void startComputation();
    void stopComputation();
    void dataSetPicked(QString);

public slots:
    void computationStopped();

private slots:
    void onStartToggled(bool pressed);
    void numIterationsChanged(const QString &value);
    void perplexityChanged(const QString &value);
    void exaggerationChanged(const QString &value);
    void expDecayChanged(const QString &value);
    void numTreesChanged(const QString &value);
    void numChecksChanged(const QString &value);
    void thetaChanged(const QString& value);

public:
    hdps::DimensionSelectionWidget _dimensionSelectionWidget;

    QComboBox dataOptions;
    QLineEdit numIterations;
    QLineEdit perplexity;
    QLineEdit exaggeration;
    QLineEdit expDecay;
    QLineEdit numTrees;
    QLineEdit numChecks;
    QLineEdit theta;
    QPushButton startButton;
};
