#include "AdvancedTsneSettingsAction.h"
#include "TsneSettingsAction.h"

#include <QTableView>

using namespace hdps::gui;

AdvancedTsneSettingsAction::AdvancedTsneSettingsAction(TsneSettingsAction& tsneSettingsAction) :
    GroupAction(&tsneSettingsAction),
    _tsneSettingsAction(tsneSettingsAction),
    _exaggerationAction(this, "Exaggeration"),
    _exponentialDecayAction(this, "Exponential decay"),
    _numTreesAction(this, "Number of trees"),
    _numChecksAction(this, "Number of checks")
{
    setText("Advanced TSNE");
    setName("Advanced");

    const auto& tsneParameters = _tsneSettingsAction.getTsneParameters();

    _exaggerationAction.setDefaultWidgetFlags(IntegralAction::SpinBox);
    _exponentialDecayAction.setDefaultWidgetFlags(IntegralAction::SpinBox);
    _numTreesAction.setDefaultWidgetFlags(IntegralAction::SpinBox);
    _numChecksAction.setDefaultWidgetFlags(IntegralAction::SpinBox);

    _exaggerationAction.initialize(1, 10000, 250, 250);
    _exponentialDecayAction.initialize(1, 10000, 70, 70);
    _numTreesAction.initialize(1, 10000, 4, 4);
    _numChecksAction.initialize(1, 10000, 1024, 1024);

    const auto updateExaggeration = [this]() -> void {
        _tsneSettingsAction.getTsneParameters().setExaggerationIter(_exaggerationAction.getValue());
    };

    const auto updateExponentialDecay = [this]() -> void {
        _tsneSettingsAction.getTsneParameters().setExponentialDecayIter(_exponentialDecayAction.getValue());
    };

    const auto updateNumTrees = [this]() -> void {
        _tsneSettingsAction.getTsneParameters().setNumTrees(_numTreesAction.getValue());
    };

    const auto updateNumChecks = [this]() -> void {
        _tsneSettingsAction.getTsneParameters().setNumChecks(_numChecksAction.getValue());
    };

    const auto isResettable = [this]() -> bool {
        if (_exaggerationAction.isResettable())
            return true;

        if (_exponentialDecayAction.isResettable())
            return true;

        if (_numTreesAction.isResettable())
            return true;

        if (_numChecksAction.isResettable())
            return true;

        return false;
    };

    const auto updateReadOnly = [this, isResettable]() -> void {
        const auto enable = !isReadOnly();

        _exaggerationAction.setEnabled(enable);
        _exponentialDecayAction.setEnabled(enable);
        _numTreesAction.setEnabled(enable);
        _numChecksAction.setEnabled(enable);
    };

    connect(&_exaggerationAction, &IntegralAction::valueChanged, this, [this, updateExaggeration](const std::int32_t& value) {
        updateExaggeration();
    });

    connect(&_exponentialDecayAction, &IntegralAction::valueChanged, this, [this, updateExponentialDecay](const std::int32_t& value) {
        updateExponentialDecay();
    });

    connect(&_numTreesAction, &IntegralAction::valueChanged, this, [this, updateNumTrees](const std::int32_t& value) {
        updateNumTrees();
    });

    connect(&_numChecksAction, &IntegralAction::valueChanged, this, [this, updateNumChecks](const std::int32_t& value) {
        updateNumChecks();
    });

    connect(this, &GroupAction::readOnlyChanged, this, [this, updateReadOnly](const bool& readOnly) {
        updateReadOnly();
    });

    updateExaggeration();
    updateExponentialDecay();
    updateNumTrees();
    updateNumChecks();
    updateReadOnly();
}
