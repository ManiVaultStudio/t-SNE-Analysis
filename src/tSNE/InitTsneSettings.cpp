#include "InitTsneSettings.h"

#include "TsneSettingsAction.h"

#include <cmath>
#include <numeric>
#include <random>
#include <utility>

constexpr auto SEEDMIN = -1000;
constexpr auto SEEDMAX =  1000;

static std::random_device rd;
static std::default_random_engine gen(rd());
static std::uniform_int_distribution<int> dst(SEEDMIN, SEEDMAX);

static inline int NewRandomSeed() { return dst(gen); }

using namespace mv::gui;

InitTsneSettings::InitTsneSettings(TsneSettingsAction& tsneSettingsAction) :
    GroupAction(&tsneSettingsAction, "Initialization", true),
    _tsneSettingsAction(tsneSettingsAction),
    _randomInitAction(this, "Random", true),
    _newSeedAction(this, "New seed", true),
    _randomSeedAction(this, "Random seed"),
    _datasetInitAction(this, "Init data"),
    _dataDimensionActionX(this, "Init dim X"),
    _dataDimensionActionY(this, "Init dim Y"),
    _rescaleInitAction(this, "Rescale", true)
{
    addAction(&_randomInitAction);
    addAction(&_randomSeedAction);
    addAction(&_newSeedAction);
    addAction(&_datasetInitAction);
    addAction(&_dataDimensionActionX);
    addAction(&_dataDimensionActionY);
    addAction(&_rescaleInitAction);

    _randomInitAction.setToolTip("Init t-SNE randomly.");
    _newSeedAction.setToolTip("Use a new random seed when re-initializing the embedding.");
    _randomSeedAction.setToolTip("Seed for random init.");
    _datasetInitAction.setToolTip("Data set to use for init.");
    _dataDimensionActionX.setToolTip("Dimensions of dataset to use for inititial embedding X dimension.");
    _dataDimensionActionY.setToolTip("Dimensions of dataset to use for inititial embedding Y dimension.");
    _rescaleInitAction.setToolTip("Whether to rescale the init embedding.");

    // always start with a random seed
    _randomSeedAction.initialize(SEEDMIN, SEEDMAX, NewRandomSeed());

    // only list point datasets with at least 2 dimensions
    _datasetInitAction.setDatasetsFilterFunction([](const mv::Datasets& datasets) -> Datasets {
        Datasets pointDatasets;

        for (const auto& dataset : datasets)
            if (dataset->getDataType() == PointType)
                if(Dataset<Points>(dataset)->getNumDimensions() >= 2)
                    pointDatasets << dataset;

        return pointDatasets;
    });

    const auto updateReadOnly = [this]() -> void {
        const auto enable = !isReadOnly();

        _randomInitAction.setEnabled(enable);
        _randomSeedAction.setEnabled(enable);
        _datasetInitAction.setEnabled(enable);
        _dataDimensionActionX.setEnabled(enable);
        _dataDimensionActionY.setEnabled(enable);
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

            return std::pair{ /* x = */ r * std::cos(t), /* y = */ r * std::sin(t) };   // conversion to cartesian coordinates
        };

        for (size_t i = 0; i < numPoints; ++i) {
            auto randomPoint = randomVec();

            initPositions[i * 2]        = randomPoint.first;
            initPositions[i * 2 + 1]    = randomPoint.second;
        }
    }
    else
    {
        auto initData = _datasetInitAction.getCurrentDataset<Points>();
        auto xDim = _dataDimensionActionX.getCurrentDimensionIndex();
        auto yDim = _dataDimensionActionY.getCurrentDimensionIndex();

        initData->populateDataForDimensions(initPositions, std::vector<int32_t>{ xDim , yDim });
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
            stdevCurrent += std::pow(initPositions[i * 2] - mean, 2);

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

void InitTsneSettings::updateSeed()
{
    _randomSeedAction.setValue(NewRandomSeed());
}


void InitTsneSettings::fromVariantMap(const QVariantMap& variantMap)
{
    GroupAction::fromVariantMap(variantMap);

    _randomInitAction.fromParentVariantMap(variantMap);
    _newSeedAction.fromParentVariantMap(variantMap);
    _randomSeedAction.fromParentVariantMap(variantMap);
    _datasetInitAction.fromParentVariantMap(variantMap);
    _dataDimensionActionX.fromParentVariantMap(variantMap);
    _dataDimensionActionY.fromParentVariantMap(variantMap);
    _rescaleInitAction.fromParentVariantMap(variantMap);
}

QVariantMap InitTsneSettings::toVariantMap() const
{
    QVariantMap variantMap = GroupAction::toVariantMap();

    _randomInitAction.insertIntoVariantMap(variantMap);
    _newSeedAction.insertIntoVariantMap(variantMap);
    _randomSeedAction.insertIntoVariantMap(variantMap);
    _datasetInitAction.insertIntoVariantMap(variantMap);
    _dataDimensionActionX.insertIntoVariantMap(variantMap);
    _dataDimensionActionY.insertIntoVariantMap(variantMap);
    _rescaleInitAction.insertIntoVariantMap(variantMap);

    return variantMap;
}
