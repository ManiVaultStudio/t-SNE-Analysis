#include "GeneralHsneSettingsAction.h"
#include "HsneAnalysisPlugin.h"

using namespace hdps::gui;

GeneralHsneSettingsAction::GeneralHsneSettingsAction(HsneSettingsAction& hsneSettingsAction) :
    WidgetActionGroup(&hsneSettingsAction, true),
    _hsneSettingsAction(hsneSettingsAction),
    _knnTypeAction(this, "KNN Type"),
    _seedAction(this, "Random seed"),
    _useMonteCarloSamplingAction(this, "Use Monte Carlo sampling")
{
    setText("HSNE");

    const auto& hsneParameters = hsneSettingsAction.getHsneParameters();

    _knnTypeAction.initialize(QStringList({ "FLANN", "HNSW", "ANNOY" }), "FLANN", "FLANN");
    _seedAction.initialize(-1000, 1000, -1, -1);
    _useMonteCarloSamplingAction.initialize(hsneParameters.useMonteCarloSampling(), hsneParameters.useMonteCarloSampling());

    const auto updateKnnAlgorithm = [this]() -> void {
        _hsneSettingsAction.getHsneParameters().setKnnLibrary(static_cast<hdi::dr::knn_library>(_knnTypeAction.getCurrentIndex()));
    };

    const auto updateSeed = [this]() -> void {
        _hsneSettingsAction.getHsneParameters().setSeed(_seedAction.getValue());
    };

    const auto updateUseMonteCarloSampling = [this]() -> void {
        _hsneSettingsAction.getHsneParameters().useMonteCarloSampling(_useMonteCarloSamplingAction.isChecked());
    };

    connect(&_knnTypeAction, &OptionAction::currentIndexChanged, this, [this, updateKnnAlgorithm]() {
        updateKnnAlgorithm();
    });

    connect(&_seedAction, &IntegralAction::valueChanged, this, [this, updateSeed]() {
        updateSeed();
    });

    connect(&_useMonteCarloSamplingAction, &ToggleAction::toggled, this, [this, updateUseMonteCarloSampling]() {
        updateUseMonteCarloSampling();
    });

    updateKnnAlgorithm();
    updateSeed();
    updateUseMonteCarloSampling();
}

void GeneralHsneSettingsAction::setReadOnly(const bool& readOnly)
{
    const auto enabled = !readOnly;

    _knnTypeAction.setEnabled(enabled);
    _seedAction.setEnabled(enabled);
    _useMonteCarloSamplingAction.setEnabled(enabled);
}

GeneralHsneSettingsAction::Widget::Widget(QWidget* parent, GeneralHsneSettingsAction* generalHsneSettingsAction, const Widget::State& state) :
    WidgetActionGroup::GroupWidget(parent, generalHsneSettingsAction)
{
    addWidgetAction(generalHsneSettingsAction->_knnTypeAction);
    addWidgetAction(generalHsneSettingsAction->_seedAction);
    addWidgetAction(generalHsneSettingsAction->_useMonteCarloSamplingAction);
    addWidgetAction(generalHsneSettingsAction->getHsneSettingsAction().getStartStopAction(), true);
}
