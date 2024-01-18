#include "GeneralHsneSettingsAction.h"

#include "HsneSettingsAction.h"

using namespace mv::gui;

GeneralHsneSettingsAction::GeneralHsneSettingsAction(HsneSettingsAction& hsneSettingsAction) :
    GroupAction(&hsneSettingsAction, "General HSNE", true),
    _hsneSettingsAction(hsneSettingsAction),
    _knnTypeAction(this, "KNN Type"),
    _numScalesAction(this, "Number of Hierarchy Scales"),
    _seedAction(this, "Random seed"),
    _useMonteCarloSamplingAction(this, "Use Monte Carlo sampling"),
    _startAction(this, "Start")
{
    setSerializationName("generalHsneSettings");

    addAction(&_knnTypeAction);
    addAction(&_numScalesAction);
    addAction(&_seedAction);
    addAction(&_useMonteCarloSamplingAction);
    addAction(&_startAction);

    const auto& hsneParameters = hsneSettingsAction.getHsneParameters();

    _knnTypeAction.setDefaultWidgetFlags(OptionAction::ComboBox);
    _numScalesAction.setDefaultWidgetFlags(IntegralAction::SpinBox);
    _seedAction.setDefaultWidgetFlags(IntegralAction::SpinBox);
    _useMonteCarloSamplingAction.setDefaultWidgetFlags(ToggleAction::CheckBox);

    _knnTypeAction.initialize(QStringList({ "FLANN", "HNSW", "ANNOY" }), "FLANN");
    _numScalesAction.initialize(1, 10, hsneParameters.getNumScales());
    _seedAction.initialize(-1000, 1000, hsneParameters.getSeed());
    _useMonteCarloSamplingAction.setChecked(hsneParameters.useMonteCarloSampling());

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

void GeneralHsneSettingsAction::setPerplexity(const int32_t& perplexity) {
    _hsneSettingsAction.getHsneParameters().setNumNearestNeighbors(perplexity * 3);
}

void GeneralHsneSettingsAction::setDistanceMetric(const hdi::dr::knn_distance_metric& metric) {
    _hsneSettingsAction.getHsneParameters().setKnnMetric(metric);
}

void GeneralHsneSettingsAction::fromVariantMap(const QVariantMap& variantMap)
{
    GroupAction::fromVariantMap(variantMap);

    _knnTypeAction.fromParentVariantMap(variantMap);
    _numScalesAction.fromParentVariantMap(variantMap);
    _seedAction.fromParentVariantMap(variantMap);
    _useMonteCarloSamplingAction.fromParentVariantMap(variantMap);
    _startAction.fromParentVariantMap(variantMap);
}

QVariantMap GeneralHsneSettingsAction::toVariantMap() const
{
    QVariantMap variantMap = GroupAction::toVariantMap();

    _knnTypeAction.insertIntoVariantMap(variantMap);
    _numScalesAction.insertIntoVariantMap(variantMap);
    _seedAction.insertIntoVariantMap(variantMap);
    _useMonteCarloSamplingAction.insertIntoVariantMap(variantMap);
    _startAction.insertIntoVariantMap(variantMap);

    return variantMap;
}
