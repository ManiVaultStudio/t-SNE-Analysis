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
    _randomInitAction(this, "Random inital embedding", true),
    _newRandomSeedAction(this, "New seed on re-initialization", true),
    _randomSeedAction(this, "Random seed"),
    _datasetInitAction(this, "Init data"),
    _dataDimensionActionX(this, "Init dim X"),
    _dataDimensionActionY(this, "Init dim Y"),
    _rescaleInitAction(this, "Rescale to small std dev", true),
    _numPointsInputData(0)
{
    addAction(&_randomInitAction);
    addAction(&_randomSeedAction);
    addAction(&_newRandomSeedAction);
    addAction(&_datasetInitAction);
    addAction(&_dataDimensionActionX);
    addAction(&_dataDimensionActionY);
    addAction(&_rescaleInitAction);

    _randomInitAction.setToolTip("Init t-SNE randomly.");
    _newRandomSeedAction.setToolTip("Use a new random seed when re-initializing the embedding.");
    _randomSeedAction.setToolTip("Seed for random init.");
    _datasetInitAction.setToolTip("Data set to use for init.");
    _dataDimensionActionX.setToolTip("Dimensions of dataset to use for inititial embedding X dimension.");
    _dataDimensionActionY.setToolTip("Dimensions of dataset to use for inititial embedding Y dimension.");
    _rescaleInitAction.setToolTip("Whether to rescale the init embedding such that the the standard deviation of \nthe first embedding dimension is 0.0001.");

    _datasetInitAction.setEnabled(false);
    _dataDimensionActionX.setEnabled(false);
    _dataDimensionActionY.setEnabled(false);

    // always start with a random seed
    _randomSeedAction.initialize(SEEDMIN, SEEDMAX, NewRandomSeed());

    updateDataPicker(0);

    connect(&_datasetInitAction, &DatasetPickerAction::datasetPicked , this, [this](mv::Dataset<mv::DatasetImpl> pickedDataset) {
        _dataDimensionActionX.setPointsDataset(pickedDataset);
        _dataDimensionActionY.setPointsDataset(pickedDataset);

        _dataDimensionActionX.setCurrentDimensionIndex(0);
        _dataDimensionActionY.setCurrentDimensionIndex(1);
    });

    connect(&_randomInitAction, &ToggleAction::toggled , this, [this](bool) {
        const auto checked = _randomInitAction.isChecked();

        _newRandomSeedAction.setEnabled(checked);
        _newRandomSeedAction.setCheckable(checked);
        _randomSeedAction.setEnabled(checked);
        _randomSeedAction.setCheckable(checked);

        _datasetInitAction.setEnabled(!checked);
        _dataDimensionActionX.setEnabled(!checked);
        _dataDimensionActionY.setEnabled(!checked);
    });

    const auto updateReadOnly = [this]() -> void {
        auto enable = !isReadOnly();

        _randomInitAction.setEnabled(enable);
        _randomSeedAction.setEnabled(enable);
        _newRandomSeedAction.setEnabled(enable);
        _rescaleInitAction.setEnabled(enable);

        if (enable && _randomInitAction.isChecked())
            enable = false;

        _datasetInitAction.setEnabled(enable);
        _dataDimensionActionX.setEnabled(enable);
        _dataDimensionActionY.setEnabled(enable);
    };

    connect(this, &GroupAction::readOnlyChanged, this, [this, updateReadOnly](const bool& readOnly) {
        updateReadOnly();
    });
}

void InitTsneSettings::updateDataPicker(size_t numPointsInputData) {
    _numPointsInputData = numPointsInputData;

    _datasetInitAction.setDatasetsFilterFunction([numPointsInput = this->_numPointsInputData](const mv::Datasets& datasets) -> Datasets {
        Datasets possibleInitDataset;

        for (const auto& dataset : datasets)
            if (dataset->getDataType() == PointType)
            {
                const auto pointDataset = Dataset<Points>(dataset);
                if (pointDataset->getNumDimensions() >= 2 && pointDataset->getNumPoints() == numPointsInput)
                    possibleInitDataset << dataset;
            }

        return possibleInitDataset;
    });
};

std::vector<float> InitTsneSettings::getInitEmbedding(size_t numPoints)
{
    assert(numPoints > 0);

    std::vector<float> initPositions(numPoints * 2, -1.f);

    if (_randomInitAction.isChecked())
    {
        qDebug() << "Initialize t-SNE embedding randomly";

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

        qDebug() << "Initialize t-SNE embedding with " << initData->getGuiName() << " using dimensions " << xDim << " and " << yDim;

        initData->populateDataForDimensions(initPositions, std::vector<int32_t>{ xDim , yDim });
    }

    if (_rescaleInitAction.isChecked())
    {
        const float stdevDesired = 0.0001f;

        qDebug() << "Rescale initial embedding to such that the the standard deviation of the its first dimension is " << stdevDesired;

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
    _newRandomSeedAction.fromParentVariantMap(variantMap);
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
    _newRandomSeedAction.insertIntoVariantMap(variantMap);
    _randomSeedAction.insertIntoVariantMap(variantMap);
    _datasetInitAction.insertIntoVariantMap(variantMap);
    _dataDimensionActionX.insertIntoVariantMap(variantMap);
    _dataDimensionActionY.insertIntoVariantMap(variantMap);
    _rescaleInitAction.insertIntoVariantMap(variantMap);

    return variantMap;
}
