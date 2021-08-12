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

    const auto updateReadOnly = [this]() -> void {
        const auto enabled = !isReadOnly();

        _knnTypeAction.setEnabled(enabled);
        _seedAction.setEnabled(enabled);
        _useMonteCarloSamplingAction.setEnabled(enabled);
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

    connect(&_hsneSettingsAction.getStartAction(), &ToggleAction::toggled, this, [this](bool toggled) {
        setReadOnly(toggled);
    });

    connect(this, &WidgetActionGroup::readOnlyChanged, this, [this, updateReadOnly](const bool& readOnly) {
        updateReadOnly();
    });

    updateKnnAlgorithm();
    updateSeed();
    updateUseMonteCarloSampling();
    updateReadOnly();
}

GeneralHsneSettingsAction::Widget::Widget(QWidget* parent, GeneralHsneSettingsAction* generalHsneSettingsAction, const Widget::State& state) :
    WidgetActionGroup::Widget(parent, generalHsneSettingsAction)
{
    addWidgetAction(generalHsneSettingsAction->_knnTypeAction);
    addWidgetAction(generalHsneSettingsAction->_seedAction);
    addWidgetAction(generalHsneSettingsAction->_useMonteCarloSamplingAction);
    addWidgetAction(generalHsneSettingsAction->getHsneSettingsAction().getStartAction(), true);
}
