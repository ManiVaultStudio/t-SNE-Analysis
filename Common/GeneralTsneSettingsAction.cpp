#include "GeneralTsneSettingsAction.h"
#include "TsneSettingsAction.h"

#include <QLabel>
#include <QPushButton>

using namespace hdps::gui;

GeneralTsneSettingsAction::GeneralTsneSettingsAction(TsneSettingsAction& tsneSettingsAction) :
    WidgetActionGroup(&tsneSettingsAction, true),
    _tsneSettingsAction(tsneSettingsAction),
    _knnTypeAction(this, "KNN Type"),
    _distanceMetricAction(this, "Distance metric"),
    _numIterationsAction(this, "Number of iterations"),
    _perplexityAction(this, "Perplexity"),
    _resetAction(this, "Reset all")
{
    setText("TSNE");

    const auto& tsneParameters = _tsneSettingsAction.getTsneParameters();

    _knnTypeAction.initialize(QStringList({ "FLANN", "HNSW", "ANNOY" }), "FLANN", "FLANN");
    _distanceMetricAction.initialize(QStringList({ "Euclidean", "Cosine", "Inner Product", "Manhattan", "Hamming", "Dot" }), "Euclidean", "Euclidean");
    _numIterationsAction.initialize(1, 10000, 1000, 1000);
    _perplexityAction.initialize(2, 50, 30, 30);

    const auto updateKnnAlgorithm = [this]() -> void {
        _tsneSettingsAction.getTsneParameters().setKnnAlgorithm(static_cast<hdi::dr::knn_library>(_knnTypeAction.getCurrentIndex()));
    };

    const auto updateDistanceMetric = [this]() -> void {
        _tsneSettingsAction.getTsneParameters().setKnnDistanceMetric(static_cast<hdi::dr::knn_distance_metric>(_distanceMetricAction.getCurrentIndex()));
    };

    const auto updateNumIterations = [this]() -> void {
        _tsneSettingsAction.getTsneParameters().setNumIterations(_numIterationsAction.getValue());
    };

    const auto updatePerplexity = [this]() -> void {
        _tsneSettingsAction.getTsneParameters().setPerplexity(_perplexityAction.getValue());
    };

    const auto canReset = [this]() -> bool {
        if (_knnTypeAction.canReset())
            return true;

        if (_distanceMetricAction.canReset())
            return true;

        if (_numIterationsAction.canReset())
            return true;

        if (_perplexityAction.canReset())
            return true;

        return false;
    };

    const auto updateReset = [this, canReset]() -> void {
        _resetAction.setEnabled(canReset());
    };

    connect(&_knnTypeAction, &OptionAction::currentIndexChanged, this, [this, updateDistanceMetric, updateReset](const std::int32_t& currentIndex) {
        updateDistanceMetric();
        updateReset();
    });

    connect(&_distanceMetricAction, &OptionAction::currentIndexChanged, this, [this, updateDistanceMetric, updateReset](const std::int32_t& currentIndex) {
        updateDistanceMetric();
        updateReset();
    });

    connect(&_numIterationsAction, &IntegralAction::valueChanged, this, [this, updateNumIterations, updateReset](const std::int32_t& value) {
        updateNumIterations();
        updateReset();
    });

    connect(&_perplexityAction, &IntegralAction::valueChanged, this, [this, updatePerplexity, updateReset](const std::int32_t& value) {
        updatePerplexity();
        updateReset();
    });

    connect(&_resetAction, &TriggerAction::triggered, this, [this](const std::int32_t& value) {
        _knnTypeAction.reset();
        _distanceMetricAction.reset();
        _numIterationsAction.reset();
        _perplexityAction.reset();
    });

    updateKnnAlgorithm();
    updateDistanceMetric();
    updateNumIterations();
    updatePerplexity();
    updateReset();
}

void GeneralTsneSettingsAction::setReadOnly(const bool& readOnly)
{
    const auto enable = !readOnly;

    _knnTypeAction.setEnabled(enable);
    _distanceMetricAction.setEnabled(enable);
    _numIterationsAction.setEnabled(enable);
    _perplexityAction.setEnabled(enable);
    _resetAction.setEnabled(enable);
}

GeneralTsneSettingsAction::Widget::Widget(QWidget* parent, GeneralTsneSettingsAction* generalTsneSettingsAction, const Widget::State& state) :
    WidgetActionGroup::GroupWidget(parent, generalTsneSettingsAction)
{
    addWidgetAction(generalTsneSettingsAction->_knnTypeAction);
    addWidgetAction(generalTsneSettingsAction->_distanceMetricAction);
    addWidgetAction(generalTsneSettingsAction->_numIterationsAction);
    addWidgetAction(generalTsneSettingsAction->_perplexityAction);
    addWidgetAction(generalTsneSettingsAction->getTsneSettingsAction().getComputationAction());
}
