#include "GeneralHsneSettingsAction.h"

#include "HsneSettingsAction.h"

using namespace mv::gui;

GeneralHsneSettingsAction::GeneralHsneSettingsAction(HsneSettingsAction& hsneSettingsAction) :
    GroupAction(&hsneSettingsAction, "HSNE", true),
    _hsneSettingsAction(hsneSettingsAction),
    _numScalesAction(this, "Hierarchy Scales"),
    _knnAlgorithmAction(this, "kNN Algorithm"),
    _distanceMetricAction(this, "Distance metric"),
    _publishLandmarkWeightAction(this, "Publish landmark weights", false),
    _numKnnAction(this, "Number of NN"),
    _startAction(this, "Start")
{
    addAction(&_numScalesAction);
    addAction(&_knnAlgorithmAction);
    addAction(&_distanceMetricAction);
    addAction(&_numKnnAction);
    addAction(&_publishLandmarkWeightAction);
    addAction(&_startAction);

    _knnAlgorithmAction.setDefaultWidgetFlags(OptionAction::ComboBox);
    _numScalesAction.setDefaultWidgetFlags(IntegralAction::SpinBox);
    _distanceMetricAction.setDefaultWidgetFlags(OptionAction::ComboBox);
    _numKnnAction.setDefaultWidgetFlags(IntegralAction::SpinBox | IntegralAction::Slider);

    _numScalesAction.initialize(1, 10, hsneSettingsAction.getHsneParameters().getNumScales());
    _knnAlgorithmAction.initialize(QStringList({ "FLANN", "HNSW", "ANNOY" }), "FLANN");
    _distanceMetricAction.initialize(QStringList({ "Euclidean", "Cosine", "Inner Product", "Manhattan", "Hamming", "Dot" }), "Euclidean");
    _numKnnAction.initialize(3, 300, 90);

    _publishLandmarkWeightAction.setToolTip("Create a second output dataset that stores the landmark weight\n(propotional to how many data points each landmark represents).");
    _numScalesAction.setToolTip("Number of hierarchy scales: e.g. 2 scales indicates one abstraction scale \nabove the data level, which is a scale itself.");
    _startAction.setToolTip("Initialize the HSNE hierarchy and create an embedding");

    const auto updateNumScales = [this]() -> void {
        _hsneSettingsAction.getHsneParameters().setNumScales(_numScalesAction.getValue());
        };

    const auto updateKnnAlgorithm = [this]() -> void {
        if (_knnAlgorithmAction.getCurrentText() == "FLANN")
            _hsneSettingsAction.getKnnParameters().setKnnAlgorithm(hdi::dr::knn_library::KNN_FLANN);

        if (_knnAlgorithmAction.getCurrentText() == "HNSW")
            _hsneSettingsAction.getKnnParameters().setKnnAlgorithm(hdi::dr::knn_library::KNN_HNSW);

        if (_knnAlgorithmAction.getCurrentText() == "ANNOY")
            _hsneSettingsAction.getKnnParameters().setKnnAlgorithm(hdi::dr::knn_library::KNN_ANNOY);
    };

    const auto updateDistanceMetric = [this]() -> void {
        if (_distanceMetricAction.getCurrentText() == "Euclidean")
            _hsneSettingsAction.getKnnParameters().setKnnDistanceMetric(hdi::dr::knn_distance_metric::KNN_METRIC_EUCLIDEAN);

        if (_distanceMetricAction.getCurrentText() == "Cosine")
            _hsneSettingsAction.getKnnParameters().setKnnDistanceMetric(hdi::dr::knn_distance_metric::KNN_METRIC_COSINE);

        if (_distanceMetricAction.getCurrentText() == "Inner Product")
            _hsneSettingsAction.getKnnParameters().setKnnDistanceMetric(hdi::dr::knn_distance_metric::KNN_METRIC_INNER_PRODUCT);

        if (_distanceMetricAction.getCurrentText() == "Manhattan")
            _hsneSettingsAction.getKnnParameters().setKnnDistanceMetric(hdi::dr::knn_distance_metric::KNN_METRIC_MANHATTAN);

        if (_distanceMetricAction.getCurrentText() == "Hamming")
            _hsneSettingsAction.getKnnParameters().setKnnDistanceMetric(hdi::dr::knn_distance_metric::KNN_METRIC_HAMMING);

        if (_distanceMetricAction.getCurrentText() == "Dot")
            _hsneSettingsAction.getKnnParameters().setKnnDistanceMetric(hdi::dr::knn_distance_metric::KNN_METRIC_DOT);
        };

    const auto updateNumKnn = [this]() -> void {
        _hsneSettingsAction.getHsneParameters().setNumNearestNeighbors(_numKnnAction.getValue());
        };

    const auto updateReadOnly = [this]() -> void {
        const auto enabled = !isReadOnly();

        _knnAlgorithmAction.setEnabled(enabled);
        _distanceMetricAction.setEnabled(enabled);
        _publishLandmarkWeightAction.setEnabled(enabled);
        _numScalesAction.setEnabled(enabled);
        _numKnnAction.setEnabled(enabled);
        _startAction.setEnabled(enabled);
    };

    connect(&_knnAlgorithmAction, &OptionAction::currentIndexChanged, this, [this, updateKnnAlgorithm]() {
        updateKnnAlgorithm();
    });

    connect(&_numScalesAction, &IntegralAction::valueChanged, this, [this, updateNumScales]() {
        updateNumScales();
    });

    connect(&_distanceMetricAction, &OptionAction::currentIndexChanged, this, [this, updateDistanceMetric](const std::int32_t& currentIndex) {
        updateDistanceMetric();
    });

    connect(&_numKnnAction, &IntegralAction::valueChanged, this, [this, updateNumKnn](const std::int32_t& value) {
        updateNumKnn();
    });

    connect(this, &GroupAction::readOnlyChanged, this, [this, updateReadOnly](const bool& readOnly) {
        updateReadOnly();
    });

    updateKnnAlgorithm();
    updateDistanceMetric();
    updateNumScales();
    updateNumKnn();
    updateReadOnly();
}

void GeneralHsneSettingsAction::fromVariantMap(const QVariantMap& variantMap)
{
    GroupAction::fromVariantMap(variantMap);

    _numScalesAction.fromParentVariantMap(variantMap);
    _knnAlgorithmAction.fromParentVariantMap(variantMap);
    _distanceMetricAction.fromParentVariantMap(variantMap);
    _publishLandmarkWeightAction.fromParentVariantMap(variantMap);
    _numKnnAction.fromParentVariantMap(variantMap);
    _startAction.fromParentVariantMap(variantMap);
}

QVariantMap GeneralHsneSettingsAction::toVariantMap() const
{
    QVariantMap variantMap = GroupAction::toVariantMap();

    _numScalesAction.insertIntoVariantMap(variantMap);
    _knnAlgorithmAction.insertIntoVariantMap(variantMap);
    _distanceMetricAction.insertIntoVariantMap(variantMap);
    _publishLandmarkWeightAction.insertIntoVariantMap(variantMap);
    _numKnnAction.insertIntoVariantMap(variantMap);
    _startAction.insertIntoVariantMap(variantMap);

    return variantMap;
}
