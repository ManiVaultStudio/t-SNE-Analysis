#include "GeneralTsneSettingsAction.h"
#include "TsneSettingsAction.h"

#include <QLabel>

using namespace hdps::gui;

GeneralTsneSettingsAction::GeneralTsneSettingsAction(TsneSettingsAction& tsneSettingsAction) :
    WidgetActionGroup(&tsneSettingsAction, true),
    _tsneSettingsAction(tsneSettingsAction),
    _knnTypeAction(this, "KNN Type", QStringList({ "FLANN", "HNSW", "ANNOY" }), "FLANN", "FLANN"),
    _distanceMetricAction(this, "Distance metric", QStringList({ "Euclidean", "Cosine", "Inner Product", "Manhattan", "Hamming", "Dot" }), "Euclidean", "Euclidean"),
    _numIterationsAction(this, "Number of iterations", 1, 10000, 1000, 1000),
    _perplexityAction(this, "Perplexity", 2, 50, 30, 30),
    _resetAction(this, "Reset all")
{
    setText("General TSNE");

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

    const auto enableControls = [this, canReset]() -> void {
        const auto isRunning = _tsneSettingsAction.getRunningAction().isChecked();
        const auto enable = !isRunning;

        _knnTypeAction.setEnabled(enable);
        _distanceMetricAction.setEnabled(enable);
        _numIterationsAction.setEnabled(enable);
        _perplexityAction.setEnabled(enable);
        _resetAction.setEnabled(enable && canReset());
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

    connect(&_tsneSettingsAction.getRunningAction(), &TriggerAction::toggled, this, [this, enableControls](bool toggled) {
        enableControls();
    });

    connect(&_resetAction, &TriggerAction::triggered, this, [this, enableControls](const std::int32_t& value) {
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
    enableControls();
}

GeneralTsneSettingsAction::Widget::Widget(QWidget* parent, GeneralTsneSettingsAction* generalTsneSettingsAction, const Widget::State& state) :
    WidgetAction::Widget(parent, generalTsneSettingsAction, state)
{
    auto layout = new QGridLayout();

    const auto addOptionActionToLayout = [this, layout](OptionAction& optionAction) -> void {
        const auto numRows = layout->rowCount();

        layout->addWidget(optionAction.createLabelWidget(this), numRows, 0);
        layout->addWidget(optionAction.createWidget(this), numRows, 1);
    };

    const auto addIntegralActionToLayout = [this, layout](IntegralAction& integralAction) -> void {
        const auto numRows = layout->rowCount();

        layout->addWidget(integralAction.createLabelWidget(this), numRows, 0);
        layout->addWidget(integralAction.createWidget(this), numRows, 1);
    };

    addOptionActionToLayout(generalTsneSettingsAction->_knnTypeAction);
    addOptionActionToLayout(generalTsneSettingsAction->_distanceMetricAction);
    addIntegralActionToLayout(generalTsneSettingsAction->_numIterationsAction);
    addIntegralActionToLayout(generalTsneSettingsAction->_perplexityAction);

    layout->addWidget(generalTsneSettingsAction->_resetAction.createWidget(this), layout->rowCount(), 1, 1, 2);

    auto computeLayout = new QHBoxLayout();

    auto& tsneSettingsAction = generalTsneSettingsAction->getTsneSettingsAction();

    computeLayout->addWidget(tsneSettingsAction.getStartComputationAction().createWidget(this));
    computeLayout->addWidget(tsneSettingsAction.getContinueComputationAction().createWidget(this));
    computeLayout->addWidget(tsneSettingsAction.getStopComputationAction().createWidget(this));

    layout->addLayout(computeLayout, layout->rowCount(), 1, 1, 2);

    switch (state)
    {
        case Widget::State::Standard:
            setLayout(layout);
            break;

        case Widget::State::Popup:
            setPopupLayout(layout);
            break;

        default:
            break;
    }
}