#pragma once

#include <widgets/SettingsWidget.h>

#include "HsneParameters.h"
#include "TsneParameters.h"
#include "DimensionSelectionWidget.h"

// Qt header files:
#include <QComboBox>
#include <QPushButton>
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>

#include <QGridLayout>

using namespace hdps::gui;

class HsneAnalysisPlugin;

/** This class serves as a container of all the UI elements that provide parameter options for HSNE */
class HsneOptions
{
public:
    HsneOptions(HsneParameters defaultParameters)
    {
        // UI Inputs
        knnOptions = new QComboBox();
        seed = new QLineEdit(QString::number(defaultParameters.getSeed()));
        useMonteCarloSampling = new QCheckBox();
        useMonteCarloSampling->setCheckState(defaultParameters.useMonteCarloSampling() ? Qt::Checked : Qt::Unchecked);
        numWalksForLandmarkSelection = new QLineEdit(QString::number(defaultParameters.getNumWalksForLandmarkSelection()));
        numWalksForLandmarkSelectionThreshold = new QLineEdit(QString::number(defaultParameters.getNumWalksForLandmarkSelectionThreshold()));
        randomWalkLength = new QLineEdit(QString::number(defaultParameters.getRandomWalkLength()));
        numWalksForAreaOfInfluence = new QLineEdit(QString::number(defaultParameters.getNumWalksForAreaOfInfluence()));
        minWalksRequired = new QLineEdit(QString::number(defaultParameters.getMinWalksRequired()));
        numChecksAknn = new QLineEdit(QString::number(defaultParameters.getNumChecksAKNN()));
        useOutOfCoreComputation = new QCheckBox();
        useOutOfCoreComputation->setCheckState(defaultParameters.useOutOfCoreComputation() ? Qt::Checked : Qt::Unchecked);
        
        // UI Labels
        useMonteCarloSamplingLabel = new QLabel("Use Monte Carlo Sampling");
        seedLabel = new QLabel("Random Seed");
        numWalksForLandmarkSelectionLabel = new QLabel("# Walks for Landmark Selection");
        numWalksForLandmarkSelectionThresholdLabel = new QLabel("# Threshold Walks for Landmark Selection");
        randomWalkLengthLabel = new QLabel("Random Walk Length");
        numWalksForAreaOfInfluenceLabel = new QLabel("# Walks for Area Of Influence");
        minWalksRequiredLabel = new QLabel("Min Walks Required");
        numChecksAknnLabel = new QLabel("# Checks for A-KNN");
        useOutOfCoreComputationLabel = new QLabel("Compute Out of Core");
    }

    // UI Parameter control
    QComboBox* knnOptions;
    QLineEdit* seed;
    QCheckBox* useMonteCarloSampling;
    QLineEdit* numWalksForLandmarkSelection;
    QLineEdit* numWalksForLandmarkSelectionThreshold;
    QLineEdit* randomWalkLength;
    QLineEdit* numWalksForAreaOfInfluence;
    QLineEdit* minWalksRequired;
    QLineEdit* numChecksAknn;
    QCheckBox* useOutOfCoreComputation;

    // Labels
    QLabel* useMonteCarloSamplingLabel;
    QLabel* seedLabel;
    QLabel* numWalksForLandmarkSelectionLabel;
    QLabel* numWalksForLandmarkSelectionThresholdLabel;
    QLabel* randomWalkLengthLabel;
    QLabel* numWalksForAreaOfInfluenceLabel;
    QLabel* minWalksRequiredLabel;
    QLabel* numChecksAknnLabel;
    QLabel* useOutOfCoreComputationLabel;
};

/** This class serves as a container of all the UI elements that provide parameter options for t-SNE */
class TsneOptions
{
public:
    TsneOptions(TsneParameters defaultParameters)
    {
        // UI Inputs
        numIterations = new QLineEdit(QString::number(defaultParameters.getNumIterations()));
        perplexity = new QLineEdit(QString::number(defaultParameters.getPerplexity()));
        numTrees = new QLineEdit(QString::number(defaultParameters.getNumTrees()));
        numChecks = new QLineEdit(QString::number(defaultParameters.getNumChecks()));
        exaggeration = new QLineEdit(QString::number(defaultParameters.getExaggerationIter()));
        expDecay = new QLineEdit(QString::number(defaultParameters.getExponentialDecayIter()));
        embeddingName = new QLineEdit("Embedding");

        // UI Labels
        numIterationsLabel = new QLabel("Iteration Count");
        perplexityLabel = new QLabel("Perplexity");
        numTreesLabel = new QLabel("Number of Trees");
        numChecksLabel = new QLabel("Number of Checks");
        exaggerationLabel = new QLabel("Exaggeration");
        expDecayLabel = new QLabel("Exponential Decay");
        embeddingNameLabel = new QLabel("Embedding Name");
    }

    // UI Parameter control
    QLineEdit* numIterations;
    QLineEdit* perplexity;
    QLineEdit* numTrees;
    QLineEdit* numChecks;
    QLineEdit* exaggeration;
    QLineEdit* expDecay;
    QLineEdit* embeddingName;

    // Labels
    QLabel* numIterationsLabel;
    QLabel* perplexityLabel;
    QLabel* numTreesLabel;
    QLabel* numChecksLabel;
    QLabel* exaggerationLabel;
    QLabel* expDecayLabel;
    QLabel* embeddingNameLabel;
};

class HsneSettingsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit HsneSettingsWidget(HsneAnalysisPlugin&);

    // Explicitly delete its copy and move member functions.
    HsneSettingsWidget(const HsneSettingsWidget&) = delete;
    HsneSettingsWidget(HsneSettingsWidget&&) = delete;
    HsneSettingsWidget& operator=(const HsneSettingsWidget&) = delete;
    HsneSettingsWidget& operator=(HsneSettingsWidget&&) = delete;

    QString getCurrentDataItem();
    void addDataItem(const QString name);
    void removeDataItem(const QString name);

    hdps::DimensionSelectionWidget& getDimensionSelectionWidget();
    HsneParameters getHsneParameters() const;
    TsneParameters getTsneParameters() const;
    QString getEmbeddingName();

signals:
    void dataSetPicked(QString);
    void startComputation();
    void stopComputation();

public slots:
    void onComputationStopped();
    void setEmbeddingName(QString embeddingName);

private slots:
    void onStartToggled(bool pressed);

private:
    //HsneParameters _parameters;

    QLineEdit _embeddingNameLine;

    QComboBox* _dataOptions;
    hdps::DimensionSelectionWidget _dimensionSelectionWidget;
    HsneOptions _hsneOptions;
    TsneOptions _tsneOptions;
    QPushButton* _startButton;

    HsneAnalysisPlugin& _analysisPlugin;
};
