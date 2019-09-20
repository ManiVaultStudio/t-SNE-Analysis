#pragma once

#include <widgets/SettingsWidget.h>

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
 * Widget containing checkboxes for enabling/disabling certain dimensions of the data.
 */
struct DimensionPickerWidget : QWidget
{
    // Explicitly delete its copy and move member functions.
    DimensionPickerWidget(const DimensionPickerWidget&) = delete;
    DimensionPickerWidget(DimensionPickerWidget&&) = delete;
    DimensionPickerWidget& operator=(const DimensionPickerWidget&) = delete;
    DimensionPickerWidget& operator=(DimensionPickerWidget&&) = delete;

    DimensionPickerWidget()
    {
        setMinimumHeight(100);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

        setLayout(&_layout);
    }

    /**
     * Returns a list of booleans that represent whether the dimension
     * at that index is enabled or not.
     *
     * @return List of enabled dimensions
     */
    std::vector<bool> getEnabledDimensions() const;

    /**
     * Adds check boxes to the widgets for every dimension.
     * Names for the dimensions can be provided. If no names are provided
     * the dimensions will be named according to their index.
     *
     * @param numDimensions Number of checkboxes to add
     * @param names         Names of the dimensions, can be an empty vector
     */
    void setDimensions(unsigned int numDimensions, const std::vector<QString>& names);

    void readSelectionFromFile(const QString&);

    void writeSelectionToFile(const QString&);

private:
    void clearWidget();

    std::vector<QCheckBox*> _checkBoxes;
    QGridLayout _layout;
    std::vector<QString> _names;
    std::unique_ptr<bool[]> _enabledDimensions;
};


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
    void onNumDimensionsChanged(TsneAnalysisPlugin* analysis, unsigned int numDimensions, const std::vector<QString>& names);
private:
    void checkInputStyle(QLineEdit& input);

signals:
    void startComputation();
    void stopComputation();
    void dataSetPicked(QString);

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
    DimensionPickerWidget _dimensionPickerWidget;

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
