#pragma once

#include <widgets/SettingsWidget.h>

#include "TsneOptions.h"
#include "PointData.h"

#include "../Common/DimensionSelectionWidget.h" // FIXME

// Qt header files:
#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QLineEdit>
#include <QLabel>
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

    explicit TsneSettingsWidget(TsneAnalysisPlugin&);

    QString getCurrentDataItem();
    void addDataItem(const QString name);
    void removeDataItem(const QString name);

    std::vector<bool> getEnabledDimensions();
    bool hasValidSettings();

    hdps::DimensionSelectionWidget& getDimensionSelectionWidget();
    TsneParameters getTsneParameters() const;

    void dataChanged(const Points& points);
    QString getEmbeddingName();

private:
    void checkInputStyle(QLineEdit* input);

signals:
    void dataSetPicked(QString);
    void knnAlgorithmPicked(int);
    void distanceMetricPicked(int);

public slots:
    void onComputationStopped();
    void setEmbeddingName(QString embName);

private slots:
    void onStartToggled(bool pressed);
    void numIterationsChanged(const QString &value);
    void perplexityChanged(const QString &value);
    void exaggerationChanged(const QString &value);
    void expDecayChanged(const QString &value);
    void numTreesChanged(const QString &value);
    void numChecksChanged(const QString &value);

public:
    QComboBox* _dataOptions;
    hdps::DimensionSelectionWidget _dimensionSelectionWidget;
    TsneOptions _tsneOptions;

    QPushButton* _startButton;

    QComboBox dataOptions;
    QLineEdit* embeddingNameLine;

private:
  TsneAnalysisPlugin& _analysisPlugin;
};
