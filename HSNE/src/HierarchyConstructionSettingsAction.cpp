#include "HierarchyConstructionSettingsAction.h"

#include "HsneSettingsAction.h"

using namespace mv::gui;

HierarchyConstructionSettingsAction::HierarchyConstructionSettingsAction(HsneSettingsAction& hsneSettingsAction) :
    GroupAction(&hsneSettingsAction, "Hierarchy Construction"),
    _hsneSettingsAction(hsneSettingsAction),
    _numWalksForLandmarkSelectionAction(this, "#walks for landmark sel."),
    _numWalksForLandmarkSelectionThresholdAction(this, "#walks for landmark sel. thres."),
    _randomWalkLengthAction(this, "Random walk length"),
    _numWalksForAreaOfInfluenceAction(this, "#walks for aoi"),
    _minWalksRequiredAction(this, "Minimum #walks required"),
    _useOutOfCoreComputationAction(this, "Out-of-core computation"),
    _useMonteCarloSamplingAction(this, "Use Monte Carlo sampling"),
    _seedAction(this, "Random seed"),
    _saveHierarchyToDiskAction(this, "Save hierarchy to disk"),
    _saveHierarchyToProjectAction(this, "Save hierarchy to project")
{
    setObjectName("Hierarchy Construction");

    addAction(&_numWalksForLandmarkSelectionAction);
    addAction(&_numWalksForLandmarkSelectionThresholdAction);
    addAction(&_randomWalkLengthAction);
    addAction(&_numWalksForAreaOfInfluenceAction);
    addAction(&_minWalksRequiredAction);
    addAction(&_seedAction);
    addAction(&_useOutOfCoreComputationAction);
    addAction(&_useMonteCarloSamplingAction);
    addAction(&_saveHierarchyToDiskAction);
    addAction(&_saveHierarchyToProjectAction);

    _numWalksForLandmarkSelectionAction.setDefaultWidgetFlags(IntegralAction::SpinBox);
    _numWalksForLandmarkSelectionThresholdAction.setDefaultWidgetFlags(IntegralAction::SpinBox);
    _randomWalkLengthAction.setDefaultWidgetFlags(IntegralAction::SpinBox);
    _numWalksForAreaOfInfluenceAction.setDefaultWidgetFlags(IntegralAction::SpinBox);
    _minWalksRequiredAction.setDefaultWidgetFlags(IntegralAction::SpinBox);
    _useOutOfCoreComputationAction.setDefaultWidgetFlags(ToggleAction::CheckBox);
    _useMonteCarloSamplingAction.setDefaultWidgetFlags(ToggleAction::CheckBox);
    _seedAction.setDefaultWidgetFlags(IntegralAction::SpinBox);
    _saveHierarchyToDiskAction.setDefaultWidgetFlags(ToggleAction::CheckBox);
    _saveHierarchyToProjectAction.setDefaultWidgetFlags(ToggleAction::CheckBox);

    _numWalksForLandmarkSelectionAction.setToolTip("Number of walks for landmark selection");
    _numWalksForLandmarkSelectionThresholdAction.setToolTip("Number of walks for landmark selection");
    _randomWalkLengthAction.setToolTip("Number of walks for landmark selection threshold");
    _numWalksForAreaOfInfluenceAction.setToolTip("Number of walks for area of influence");
    _minWalksRequiredAction.setToolTip("Minimum number of walks required");
    _useOutOfCoreComputationAction.setToolTip("Use out-of-core computation");
    _useMonteCarloSamplingAction.setToolTip("Use Monte Carlo Sampling");
    _seedAction.setToolTip("Random seed for initialization");
    _saveHierarchyToDiskAction.setToolTip("Save computed hierarchy to disk. \nWhen computing HSNE again with the same settings, \nthe hierarchy is loaded instead of recomputed");
    _saveHierarchyToProjectAction.setToolTip("Save computed hierarchy when saving a project. \nThis enables selection refinements \nafter loading projects");

    const auto& hsneParameters = hsneSettingsAction.getHsneParameters();

    _numWalksForLandmarkSelectionAction.initialize(1, 1000, hsneParameters.getNumWalksForLandmarkSelection());
    _numWalksForLandmarkSelectionThresholdAction.initialize(0, 1000, hsneParameters.getNumWalksForLandmarkSelectionThreshold());
    _randomWalkLengthAction.initialize(1, 100, hsneParameters.getRandomWalkLength());
    _numWalksForAreaOfInfluenceAction.initialize(1, 500, hsneParameters.getNumWalksForAreaOfInfluence());
    _minWalksRequiredAction.initialize(0, 100, hsneParameters.getMinWalksRequired());
    _useOutOfCoreComputationAction.setChecked(hsneParameters.useOutOfCoreComputation());
    _useMonteCarloSamplingAction.setChecked(hsneParameters.useMonteCarloSampling());
    _seedAction.initialize(-1000, 1000, hsneParameters.getSeed());
    _saveHierarchyToDiskAction.setChecked(hsneParameters.getSaveHierarchyToDisk());
    _saveHierarchyToProjectAction.setChecked(true);
    
    collapse();

    const auto updateNumWalksForLandmarkSelectionAction = [this]() -> void {
        _hsneSettingsAction.getHsneParameters().setNumWalksForLandmarkSelection(_numWalksForLandmarkSelectionAction.getValue());
    };

    const auto updateNumWalksForLandmarkSelectionThreshold = [this]() -> void {
        _hsneSettingsAction.getHsneParameters().setNumWalksForLandmarkSelectionThreshold(_numWalksForLandmarkSelectionThresholdAction.getValue());
    };

    const auto updateRandomWalkLength = [this]() -> void {
        _hsneSettingsAction.getHsneParameters().setRandomWalkLength(_randomWalkLengthAction.getValue());
    };

    const auto updateNumWalksForAreaOfInfluence = [this]() -> void {
        _hsneSettingsAction.getHsneParameters().setNumWalksForAreaOfInfluence(_numWalksForAreaOfInfluenceAction.getValue());
    };

    const auto updateMinWalksRequired = [this]() -> void {
        _hsneSettingsAction.getHsneParameters().setMinWalksRequired(_minWalksRequiredAction.getValue());
    };

    const auto updateSeed = [this]() -> void {
        _hsneSettingsAction.getHsneParameters().setSeed(_seedAction.getValue());
        };

    const auto updateUseMonteCarloSampling = [this]() -> void {
        _hsneSettingsAction.getHsneParameters().useMonteCarloSampling(_useMonteCarloSamplingAction.isChecked());
        };

    const auto updateUseOutOfCoreComputation = [this]() -> void {
        _hsneSettingsAction.getHsneParameters().useOutOfCoreComputation(_useOutOfCoreComputationAction.isChecked());
    };

    const auto updateSaveHierarchyToDiskAction = [this]() -> void {
        _hsneSettingsAction.getHsneParameters().setSaveHierarchyToDisk(_saveHierarchyToDiskAction.isChecked());
    };

    const auto updateReadOnly = [this]() -> void {
        const auto enabled = !isReadOnly();

        _numWalksForLandmarkSelectionAction.setEnabled(enabled);
        _numWalksForLandmarkSelectionThresholdAction.setEnabled(enabled);
        _randomWalkLengthAction.setEnabled(enabled);
        _numWalksForAreaOfInfluenceAction.setEnabled(enabled);
        _minWalksRequiredAction.setEnabled(enabled);
        _useOutOfCoreComputationAction.setEnabled(enabled);
        _useMonteCarloSamplingAction.setEnabled(enabled);
        _seedAction.setEnabled(enabled);
    };

    connect(&_numWalksForLandmarkSelectionAction, &IntegralAction::valueChanged, this, [this, updateNumWalksForLandmarkSelectionAction]() {
        updateNumWalksForLandmarkSelectionAction();
    });

    connect(&_numWalksForLandmarkSelectionThresholdAction, &DecimalAction::valueChanged, this, [this, updateNumWalksForLandmarkSelectionThreshold]() {
        updateNumWalksForLandmarkSelectionThreshold();
    });

    connect(&_randomWalkLengthAction, &IntegralAction::valueChanged, this, [this, updateRandomWalkLength]() {
        updateRandomWalkLength();
    });

    connect(&_numWalksForAreaOfInfluenceAction, &IntegralAction::valueChanged, this, [this, updateNumWalksForAreaOfInfluence]() {
        updateNumWalksForAreaOfInfluence();
    });

    connect(&_minWalksRequiredAction, &IntegralAction::valueChanged, this, [this, updateMinWalksRequired]() {
        updateMinWalksRequired();
    });

    connect(&_useOutOfCoreComputationAction, &ToggleAction::toggled, this, [this, updateUseOutOfCoreComputation]() {
        updateUseOutOfCoreComputation();
    });

    connect(&_useMonteCarloSamplingAction, &ToggleAction::toggled, this, [this, updateUseMonteCarloSampling]() {
        updateUseMonteCarloSampling();
    });

    connect(&_seedAction, &IntegralAction::valueChanged, this, [this, updateSeed]() {
        updateSeed();
    });

    connect(&_saveHierarchyToDiskAction, &ToggleAction::toggled, this, [this, updateSaveHierarchyToDiskAction]() {
        updateSaveHierarchyToDiskAction();
    });

    connect(this, &GroupAction::readOnlyChanged, this, [this, updateReadOnly](const bool& readOnly) {
        updateReadOnly();
    });

    updateNumWalksForLandmarkSelectionAction();
    updateNumWalksForLandmarkSelectionThreshold();
    updateRandomWalkLength();
    updateNumWalksForAreaOfInfluence();
    updateMinWalksRequired();
    updateUseOutOfCoreComputation();
    updateUseMonteCarloSampling();
    updateSeed();
    updateSaveHierarchyToDiskAction();
    updateReadOnly();
}

void HierarchyConstructionSettingsAction::fromVariantMap(const QVariantMap& variantMap)
{
    GroupAction::fromVariantMap(variantMap);

    _numWalksForLandmarkSelectionAction.fromParentVariantMap(variantMap);
    _numWalksForLandmarkSelectionThresholdAction.fromParentVariantMap(variantMap);
    _randomWalkLengthAction.fromParentVariantMap(variantMap);
    _numWalksForAreaOfInfluenceAction.fromParentVariantMap(variantMap);
    _minWalksRequiredAction.fromParentVariantMap(variantMap);
    _useMonteCarloSamplingAction.fromParentVariantMap(variantMap);
    _useOutOfCoreComputationAction.fromParentVariantMap(variantMap);
    _seedAction.fromParentVariantMap(variantMap);
    _saveHierarchyToDiskAction.fromParentVariantMap(variantMap);
    _saveHierarchyToProjectAction.fromParentVariantMap(variantMap);
}

QVariantMap HierarchyConstructionSettingsAction::toVariantMap() const
{
    QVariantMap variantMap = GroupAction::toVariantMap();

    _numWalksForLandmarkSelectionAction.insertIntoVariantMap(variantMap);
    _numWalksForLandmarkSelectionThresholdAction.insertIntoVariantMap(variantMap);
    _randomWalkLengthAction.insertIntoVariantMap(variantMap);
    _numWalksForAreaOfInfluenceAction.insertIntoVariantMap(variantMap);
    _minWalksRequiredAction.insertIntoVariantMap(variantMap);
    _useMonteCarloSamplingAction.insertIntoVariantMap(variantMap);
    _useOutOfCoreComputationAction.insertIntoVariantMap(variantMap);
    _seedAction.insertIntoVariantMap(variantMap);
    _saveHierarchyToDiskAction.insertIntoVariantMap(variantMap);
    _saveHierarchyToProjectAction.insertIntoVariantMap(variantMap);

    return variantMap;
}
