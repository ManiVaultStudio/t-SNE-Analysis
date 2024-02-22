#include "InitTsneSettings.h"

#include "TsneSettingsAction.h"

#include <cmath>
#include <numeric>
#include <random>
#include <utility>

using namespace mv::gui;

InitTsneSettings::InitTsneSettings(TsneSettingsAction& tsneSettingsAction) :
    GroupAction(&tsneSettingsAction, "TSNE", true),
    _tsneSettingsAction(tsneSettingsAction),
    _randomInitAction(this, "Random", true),
    _randomSeedAction(this, "Random seed"),
    _dataSetInitAction(this, "Init data"),
    _dataDimensionAction(this, "Data dims"),
    _rescaleInitAction(this, "Rescale", true)
{
    addAction(&_randomInitAction);
    addAction(&_randomSeedAction);
    addAction(&_dataSetInitAction);
    addAction(&_dataDimensionAction);
    addAction(&_rescaleInitAction);

    _randomInitAction.setToolTip("Init t-SNE randomly.");
    _randomSeedAction.setToolTip("Seed for random init.");
    _dataSetInitAction.setToolTip("Data set to use for init.");
    _dataDimensionAction.setToolTip("Dimensions of dataset to use for init.");
    _rescaleInitAction.setToolTip("Whether to rescale the init embedding.");

    std::random_device rd;
    std::default_random_engine gen(rd());
    std::uniform_int_distribution<int> dst(-1000, 1000);

    _randomSeedAction.initialize(-1000, 1000, dst(gen));
    _dataSetInitAction.initialize();

    const auto updateReadOnly = [this]() -> void {
        const auto enable = !isReadOnly();

        _randomInitAction.setEnabled(enable);
        _randomSeedAction.setEnabled(enable);
        _dataSetInitAction.setEnabled(enable);
        _dataDimensionAction.setEnabled(enable);
        _rescaleInitAction.setEnabled(enable);
    };

    connect(this, &GroupAction::readOnlyChanged, this, [this, updateReadOnly](const bool& readOnly) {
        updateReadOnly();
    });

    updateReadOnly();
}

std::vector<float> InitTsneSettings::getInitEmbedding(size_t numPoints)
{
    assert(numPoints > 0);

    std::vector<float> initPositions(numPoints * 2, -1.f);

    if (_randomInitAction.isChecked())
    {
        std::default_random_engine gen(_randomSeedAction.getValue());
        std::uniform_real_distribution<float> dis(0, 1);

        auto randomVec = [&gen, &dis]() -> std::pair<float, float> {

            const float r = std::sqrt(dis(gen));            // random radius: uniformly sample from [0, 1], sqrt (important!)
            const float t = 2.0f * 3.141592f * dis(gen);    // random angle: uniformly sample from [0, 1] and scale to [0, 2pi]

            return std::pair{ /* x = */ r * std::cos(t), /* y = */ r * std::sin(t) };
        };

        for (size_t i = 0; i < numPoints; ++i) {
            auto randomPoint = randomVec();

            initPositions[i * 2]        = randomPoint.first;
            initPositions[i * 2 + 1]    = randomPoint.second;
        }
    }
    else
    {
        // TODO use forst two dimensions of picked data set
    }

    if (_rescaleInitAction.isChecked())
    {
        float stdevDesired = 0.0001f;

        // Calculate the mean and standard deviation of the first embedding dimension
        float sum = 0.f;
        for (size_t i = 0; i < numPoints; ++i)
            sum += initPositions[i * 2];

        float mean = sum / numPoints;

        float stdevCurrent = 0.f;
        for (size_t i = 0; i < numPoints; ++i)
            stdevCurrent += std::powf(initPositions[i * 2] - mean, 2);

        stdevCurrent = std::sqrt(stdevCurrent / numPoints);

        // Re-scale the data to match the desired standard deviation
        float scaleFactor = stdevDesired / stdevCurrent;
        for (size_t i = 0; i < numPoints; ++i) {
            initPositions[i * 2]        = (initPositions[i * 2] - mean) * scaleFactor + mean;
            initPositions[i * 2 + 1]    = (initPositions[i * 2 + 1] - mean) * scaleFactor + mean;
        }
    }

    return initPositions;
}


void InitTsneSettings::fromVariantMap(const QVariantMap& variantMap)
{
    GroupAction::fromVariantMap(variantMap);

    _randomInitAction.fromParentVariantMap(variantMap);
    _randomSeedAction.fromParentVariantMap(variantMap);
    _dataSetInitAction.fromParentVariantMap(variantMap);
    _dataDimensionAction.fromParentVariantMap(variantMap);
    _rescaleInitAction.fromParentVariantMap(variantMap);
}

QVariantMap InitTsneSettings::toVariantMap() const
{
    QVariantMap variantMap = GroupAction::toVariantMap();

    _randomInitAction.insertIntoVariantMap(variantMap);
    _randomSeedAction.insertIntoVariantMap(variantMap);
    _dataSetInitAction.insertIntoVariantMap(variantMap);
    _dataDimensionAction.insertIntoVariantMap(variantMap);
    _rescaleInitAction.insertIntoVariantMap(variantMap);

    return variantMap;
}
