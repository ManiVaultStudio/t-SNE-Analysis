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
    setText("General TSNE");

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

    //layout->addWidget(generalTsneSettingsAction->_resetAction.createWidget(this), layout->rowCount(), 1, 1, 2);

    auto& tsneSettingsAction = generalTsneSettingsAction->getTsneSettingsAction();

    auto startPushButton    = dynamic_cast<TriggerAction::Widget*>(tsneSettingsAction.getStartComputationAction().createWidget(this));
    auto continuePushButton = dynamic_cast<TriggerAction::Widget*>(tsneSettingsAction.getContinueComputationAction().createWidget(this));
    auto stopPushButton     = dynamic_cast<TriggerAction::Widget*>(tsneSettingsAction.getStopComputationAction().createWidget(this));

    /*
    startPushButton->getPushButton()->setText("");
    continuePushButton->getPushButton()->setText("");
    stopPushButton->getPushButton()->setText("");
    */

    auto computeLayout = new QHBoxLayout();

    computeLayout->addWidget(startPushButton);
    computeLayout->addWidget(continuePushButton);
    computeLayout->addWidget(stopPushButton);

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