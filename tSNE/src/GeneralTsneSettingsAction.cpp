#include "GeneralTsneSettingsAction.h"
#include "TsneSettingsAction.h"

using namespace mv::gui;

GeneralTsneSettingsAction::GeneralTsneSettingsAction(TsneSettingsAction& tsneSettingsAction) :
    GroupAction(&tsneSettingsAction, "TSNE", true),
    _tsneSettingsAction(tsneSettingsAction),
    _knnAlgorithmAction(this, "kNN Algorithm"),
    _distanceMetricAction(this, "Distance metric"),
    _numIterationsAction(this, "Number of iterations"),
    _numberOfComputatedIterationsAction(this, "Number of computed iterations", 0, 1000000000, 0),
    _perplexityAction(this, "Perplexity"),
    _updateIterationsAction(this, "Core update every"),
    _computationAction(this)
{
    addAction(&_knnAlgorithmAction);
    addAction(&_distanceMetricAction);
    addAction(&_perplexityAction);
    addAction(&_numIterationsAction);
    addAction(&_numberOfComputatedIterationsAction);
//    addAction(&_updateIterationsAction);
    addAction(&_computationAction);

    const auto& tsneParameters = _tsneSettingsAction.getTsneParameters();

    _numberOfComputatedIterationsAction.setEnabled(false);

    _knnAlgorithmAction.setDefaultWidgetFlags(OptionAction::ComboBox);
    _distanceMetricAction.setDefaultWidgetFlags(OptionAction::ComboBox);
    _numIterationsAction.setDefaultWidgetFlags(IntegralAction::SpinBox);
    _numberOfComputatedIterationsAction.setDefaultWidgetFlags(IntegralAction::LineEdit);
    _perplexityAction.setDefaultWidgetFlags(IntegralAction::SpinBox | IntegralAction::Slider);
    _updateIterationsAction.setDefaultWidgetFlags(IntegralAction::SpinBox | IntegralAction::Slider);

    _knnAlgorithmAction.initialize(QStringList({ "FLANN", "HNSW", "ANNOY" }), "FLANN");
    _distanceMetricAction.initialize(QStringList({ "Euclidean", "Cosine", "Inner Product", "Manhattan", "Hamming", "Dot" }), "Euclidean");
    _numIterationsAction.initialize(1, 10000, 1000);
    _perplexityAction.initialize(2, 50, 30);
    _updateIterationsAction.initialize(0, 10000, 10);

    _updateIterationsAction.setToolTip("Update the dataset every x iterations. If set to 0, there will be no intermediate result.");

    const auto updateKnnAlgorithm = [this]() -> void {
        if (_knnAlgorithmAction.getCurrentText() == "FLANN")
            _tsneSettingsAction.getKnnParameters().setKnnAlgorithm(hdi::dr::knn_library::KNN_FLANN);

        if (_knnAlgorithmAction.getCurrentText() == "HNSW")
            _tsneSettingsAction.getKnnParameters().setKnnAlgorithm(hdi::dr::knn_library::KNN_HNSW);

        if (_knnAlgorithmAction.getCurrentText() == "ANNOY")
            _tsneSettingsAction.getKnnParameters().setKnnAlgorithm(hdi::dr::knn_library::KNN_ANNOY);
    };

    const auto updateDistanceMetric = [this]() -> void {
        if (_distanceMetricAction.getCurrentText() == "Euclidean")
            _tsneSettingsAction.getKnnParameters().setKnnDistanceMetric(hdi::dr::knn_distance_metric::KNN_METRIC_EUCLIDEAN);

        if (_distanceMetricAction.getCurrentText() == "Cosine")
            _tsneSettingsAction.getKnnParameters().setKnnDistanceMetric(hdi::dr::knn_distance_metric::KNN_METRIC_COSINE);

        if (_distanceMetricAction.getCurrentText() == "Inner Product")
            _tsneSettingsAction.getKnnParameters().setKnnDistanceMetric(hdi::dr::knn_distance_metric::KNN_METRIC_INNER_PRODUCT);

        if (_distanceMetricAction.getCurrentText() == "Manhattan")
            _tsneSettingsAction.getKnnParameters().setKnnDistanceMetric(hdi::dr::knn_distance_metric::KNN_METRIC_MANHATTAN);

        if (_distanceMetricAction.getCurrentText() == "Hamming")
            _tsneSettingsAction.getKnnParameters().setKnnDistanceMetric(hdi::dr::knn_distance_metric::KNN_METRIC_HAMMING);

        if (_distanceMetricAction.getCurrentText() == "Dot")
            _tsneSettingsAction.getKnnParameters().setKnnDistanceMetric(hdi::dr::knn_distance_metric::KNN_METRIC_DOT);
    };

    const auto updateNumIterations = [this]() -> void {
        _tsneSettingsAction.getTsneParameters().setNumIterations(_numIterationsAction.getValue());
    };

    const auto updatePerplexity = [this]() -> void {
        _tsneSettingsAction.getTsneParameters().setPerplexity(_perplexityAction.getValue());
    };

    const auto updateCoreUpdate = [this]() -> void {
        _tsneSettingsAction.getTsneParameters().setUpdateCore(_updateIterationsAction.getValue());
    };

    const auto isResettable = [this]() -> bool {
        if (_knnAlgorithmAction.isResettable())
            return true;

        if (_distanceMetricAction.isResettable())
            return true;

        if (_numIterationsAction.isResettable())
            return true;

        if (_perplexityAction.isResettable())
            return true;

        if (_updateIterationsAction.isResettable())
            return true;

        return false;
    };

    const auto updateReadOnly = [this]() -> void {
        const auto enable = !isReadOnly();

        _knnAlgorithmAction.setEnabled(enable);
        _distanceMetricAction.setEnabled(enable);
        _numIterationsAction.setEnabled(enable);
        _perplexityAction.setEnabled(enable);
        _updateIterationsAction.setEnabled(enable);
    };

    connect(&_knnAlgorithmAction, &OptionAction::currentIndexChanged, this, [this, updateKnnAlgorithm](const std::int32_t& currentIndex) {
        updateKnnAlgorithm();
    });

    connect(&_distanceMetricAction, &OptionAction::currentIndexChanged, this, [this, updateDistanceMetric](const std::int32_t& currentIndex) {
        updateDistanceMetric();
    });

    connect(&_numIterationsAction, &IntegralAction::valueChanged, this, [this, updateNumIterations](const std::int32_t& value) {
        updateNumIterations();
    });

    connect(&_perplexityAction, &IntegralAction::valueChanged, this, [this, updatePerplexity](const std::int32_t& value) {
        updatePerplexity();
    });

    connect(&_updateIterationsAction, &IntegralAction::valueChanged, this, [this, updateCoreUpdate](const std::int32_t& value) {
        updateCoreUpdate();
    });

    connect(this, &GroupAction::readOnlyChanged, this, [this, updateReadOnly](const bool& readOnly) {
        updateReadOnly();
    });

    updateKnnAlgorithm();
    updateDistanceMetric();
    updateNumIterations();
    updatePerplexity();
    updateCoreUpdate();
    updateReadOnly();
}

void GeneralTsneSettingsAction::fromVariantMap(const QVariantMap& variantMap)
{
    GroupAction::fromVariantMap(variantMap);

    _knnAlgorithmAction.fromParentVariantMap(variantMap);
    _distanceMetricAction.fromParentVariantMap(variantMap);
    _numIterationsAction.fromParentVariantMap(variantMap);
    _numberOfComputatedIterationsAction.fromParentVariantMap(variantMap);
    _perplexityAction.fromParentVariantMap(variantMap);
    _updateIterationsAction.fromParentVariantMap(variantMap);
    _computationAction.fromParentVariantMap(variantMap);
}

QVariantMap GeneralTsneSettingsAction::toVariantMap() const
{
    qDebug() << "GeneralTsneSettingsAction::toVariantMap:";

    QVariantMap variantMap = GroupAction::toVariantMap();

    _knnAlgorithmAction.insertIntoVariantMap(variantMap);
    _distanceMetricAction.insertIntoVariantMap(variantMap);
    _numIterationsAction.insertIntoVariantMap(variantMap);
    _numberOfComputatedIterationsAction.insertIntoVariantMap(variantMap);
    _perplexityAction.insertIntoVariantMap(variantMap);
    _updateIterationsAction.insertIntoVariantMap(variantMap);
    _computationAction.insertIntoVariantMap(variantMap);

    return variantMap;
}
