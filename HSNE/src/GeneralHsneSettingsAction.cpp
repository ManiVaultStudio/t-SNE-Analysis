#include "GeneralHsneSettingsAction.h"
#include "HsneAnalysisPlugin.h"

using namespace hdps::gui;

GeneralHsneSettingsAction::GeneralHsneSettingsAction(HsneSettingsAction& hsneSettingsAction) :
    GroupAction(&hsneSettingsAction, true),
    _hsneSettingsAction(hsneSettingsAction),
    _knnTypeAction(this, "KNN Type"),
    _numScalesAction(this, "Number of Hierarchy Scales"),
    _seedAction(this, "Random seed"),
    _useMonteCarloSamplingAction(this, "Use Monte Carlo sampling"),
    _startAction(this, "Start")
{
    setText("HSNE");

    const auto& hsneParameters = hsneSettingsAction.getHsneParameters();

    _knnTypeAction.setDefaultWidgetFlags(OptionAction::ComboBox);
    _numScalesAction.setDefaultWidgetFlags(IntegralAction::SpinBox);
    _seedAction.setDefaultWidgetFlags(IntegralAction::SpinBox);
    _useMonteCarloSamplingAction.setDefaultWidgetFlags(ToggleAction::CheckBox);

    _knnTypeAction.initialize(QStringList({ "FLANN", "HNSW", "ANNOY" }), "FLANN", "FLANN");
    _numScalesAction.initialize(1, 10, 3, 3);
    _seedAction.initialize(-1000, 1000, -1, -1);
    _useMonteCarloSamplingAction.initialize(hsneParameters.useMonteCarloSampling(), hsneParameters.useMonteCarloSampling());
    
    _startAction.setToolTip("Initialize the HSNE hierarchy and create an embedding");

    const auto updateKnnAlgorithm = [this]() -> void {
        if (_knnTypeAction.getCurrentText() == "FLANN")
            _hsneSettingsAction.getHsneParameters().setKnnLibrary(hdi::dr::knn_library::KNN_FLANN);

        if (_knnTypeAction.getCurrentText() == "HNSW")
            _hsneSettingsAction.getHsneParameters().setKnnLibrary(hdi::dr::knn_library::KNN_HNSW);

        if (_knnTypeAction.getCurrentText() == "ANNOY")
            _hsneSettingsAction.getHsneParameters().setKnnLibrary(hdi::dr::knn_library::KNN_ANNOY);
    };

    const auto updateNumScales = [this]() -> void {
        _hsneSettingsAction.getHsneParameters().setNumScales(_numScalesAction.getValue());
    };

    const auto updateSeed = [this]() -> void {
        _hsneSettingsAction.getHsneParameters().setSeed(_seedAction.getValue());
    };

    const auto updateUseMonteCarloSampling = [this]() -> void {
        _hsneSettingsAction.getHsneParameters().useMonteCarloSampling(_useMonteCarloSamplingAction.isChecked());
    };

    const auto updateReadOnly = [this]() -> void {
        const auto enabled = !isReadOnly();

        _startAction.setEnabled(enabled);
        _knnTypeAction.setEnabled(enabled);
        _numScalesAction.setEnabled(enabled);
        _seedAction.setEnabled(enabled);
        _useMonteCarloSamplingAction.setEnabled(enabled);
    };

    connect(&_knnTypeAction, &OptionAction::currentIndexChanged, this, [this, updateKnnAlgorithm]() {
        updateKnnAlgorithm();
    });

    connect(&_numScalesAction, &IntegralAction::valueChanged, this, [this, updateNumScales]() {
        updateNumScales();
    });

    connect(&_seedAction, &IntegralAction::valueChanged, this, [this, updateSeed]() {
        updateSeed();
    });

    connect(&_useMonteCarloSamplingAction, &ToggleAction::toggled, this, [this, updateUseMonteCarloSampling]() {
        updateUseMonteCarloSampling();
    });

    connect(&_startAction, &ToggleAction::toggled, this, [this](bool toggled) {
        setReadOnly(toggled);
    });

    connect(this, &GroupAction::readOnlyChanged, this, [this, updateReadOnly](const bool& readOnly) {
        updateReadOnly();
    });

    updateKnnAlgorithm();
    updateNumScales();
    updateSeed();
    updateUseMonteCarloSampling();
    updateReadOnly();
}
