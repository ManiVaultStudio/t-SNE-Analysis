#include "GeneralTsneSettingsAction.h"
#include "TsneSettingsAction.h"

#include <QLabel>
#include <QPushButton>
#include <QGridLayout>

using namespace hdps::gui;

GeneralTsneSettingsAction::GeneralTsneSettingsAction(TsneSettingsAction& tsneSettingsAction) :
    GroupAction(&tsneSettingsAction, "TSNE", true),
    _tsneSettingsAction(tsneSettingsAction),
    _knnTypeAction(this, "KNN Type"),
    _distanceMetricAction(this, "Distance metric"),
    _numIterationsAction(this, "Number of iterations"),
    _numberOfComputatedIterationsAction(this, "Number of computed iterations", 0, 1000000000, 0),
    _perplexityAction(this, "Perplexity"),
    _computationAction(this)
{
    setText("TSNE");
    setObjectName("General TSNE");

    addAction(&_knnTypeAction);
    addAction(&_distanceMetricAction);
    addAction(&_numIterationsAction);
    addAction(&_numberOfComputatedIterationsAction);
    addAction(&_perplexityAction);
    addAction(&_computationAction);

    const auto& tsneParameters = _tsneSettingsAction.getTsneParameters();

    _numberOfComputatedIterationsAction.setEnabled(false);

    _knnTypeAction.setDefaultWidgetFlags(OptionAction::ComboBox);
    _distanceMetricAction.setDefaultWidgetFlags(OptionAction::ComboBox);
    _numIterationsAction.setDefaultWidgetFlags(IntegralAction::SpinBox);
    _numberOfComputatedIterationsAction.setDefaultWidgetFlags(IntegralAction::LineEdit);
    _perplexityAction.setDefaultWidgetFlags(IntegralAction::SpinBox | IntegralAction::Slider);

    _knnTypeAction.initialize(QStringList({ "FLANN", "HNSW", "ANNOY" }), "FLANN");
    _distanceMetricAction.initialize(QStringList({ "Euclidean", "Cosine", "Inner Product", "Manhattan", "Hamming", "Dot" }), "Euclidean");
    _numIterationsAction.initialize(1, 10000, 1000);
    _perplexityAction.initialize(2, 50, 30);

    const auto updateKnnAlgorithm = [this]() -> void {
        if (_knnTypeAction.getCurrentText() == "FLANN")
            _tsneSettingsAction.getTsneParameters().setKnnAlgorithm(hdi::dr::knn_library::KNN_FLANN);

        if (_knnTypeAction.getCurrentText() == "HNSW")
            _tsneSettingsAction.getTsneParameters().setKnnAlgorithm(hdi::dr::knn_library::KNN_HNSW);

        if (_knnTypeAction.getCurrentText() == "ANNOY")
            _tsneSettingsAction.getTsneParameters().setKnnAlgorithm(hdi::dr::knn_library::KNN_ANNOY);
    };

    const auto updateDistanceMetric = [this]() -> void {
        if (_distanceMetricAction.getCurrentText() == "Euclidean")
            _tsneSettingsAction.getTsneParameters().setKnnDistanceMetric(hdi::dr::knn_distance_metric::KNN_METRIC_EUCLIDEAN);

        if (_distanceMetricAction.getCurrentText() == "Cosine")
            _tsneSettingsAction.getTsneParameters().setKnnDistanceMetric(hdi::dr::knn_distance_metric::KNN_METRIC_COSINE);

        if (_distanceMetricAction.getCurrentText() == "Inner Product")
            _tsneSettingsAction.getTsneParameters().setKnnDistanceMetric(hdi::dr::knn_distance_metric::KNN_METRIC_INNER_PRODUCT);

        if (_distanceMetricAction.getCurrentText() == "Manhattan")
            _tsneSettingsAction.getTsneParameters().setKnnDistanceMetric(hdi::dr::knn_distance_metric::KNN_METRIC_MANHATTAN);

        if (_distanceMetricAction.getCurrentText() == "Hamming")
            _tsneSettingsAction.getTsneParameters().setKnnDistanceMetric(hdi::dr::knn_distance_metric::KNN_METRIC_HAMMING);

        if (_distanceMetricAction.getCurrentText() == "Dot")
            _tsneSettingsAction.getTsneParameters().setKnnDistanceMetric(hdi::dr::knn_distance_metric::KNN_METRIC_DOT);
    };

    const auto updateNumIterations = [this]() -> void {
        _tsneSettingsAction.getTsneParameters().setNumIterations(_numIterationsAction.getValue());
    };

    const auto updatePerplexity = [this]() -> void {
        _tsneSettingsAction.getTsneParameters().setPerplexity(_perplexityAction.getValue());
    };

    const auto isResettable = [this]() -> bool {
        if (_knnTypeAction.isResettable())
            return true;

        if (_distanceMetricAction.isResettable())
            return true;

        if (_numIterationsAction.isResettable())
            return true;

        if (_perplexityAction.isResettable())
            return true;

        return false;
    };

    const auto updateReadOnly = [this]() -> void {
        const auto enable = !isReadOnly();

        _knnTypeAction.setEnabled(enable);
        _distanceMetricAction.setEnabled(enable);
        _numIterationsAction.setEnabled(enable);
        _perplexityAction.setEnabled(enable);
    };

    connect(&_knnTypeAction, &OptionAction::currentIndexChanged, this, [this, updateKnnAlgorithm](const std::int32_t& currentIndex) {
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

    connect(this, &GroupAction::readOnlyChanged, this, [this, updateReadOnly](const bool& readOnly) {
        updateReadOnly();
    });

    updateKnnAlgorithm();
    updateDistanceMetric();
    updateNumIterations();
    updatePerplexity();
    updateReadOnly();
}
