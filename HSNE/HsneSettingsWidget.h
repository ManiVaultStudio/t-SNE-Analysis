#pragma once

#include "HsneAnalysis.h"

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>

#include <QGridLayout>

class HsneOptions
{
public:
    HsneOptions(HsneParameters defaultParameters)
    {
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

class HsneSettingsWidget : public QWidget
{
    Q_OBJECT
public:
    HsneSettingsWidget() :
        _options(_parameters)
    {
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

        setLayout(coreSettingsLayout);
    }

    // Explicitly delete its copy and move member functions.
    HsneSettingsWidget(const HsneSettingsWidget&) = delete;
    HsneSettingsWidget(HsneSettingsWidget&&) = delete;
    HsneSettingsWidget& operator=(const HsneSettingsWidget&) = delete;
    HsneSettingsWidget& operator=(HsneSettingsWidget&&) = delete;

private:
    HsneParameters _parameters;

    HsneOptions _options;
};
