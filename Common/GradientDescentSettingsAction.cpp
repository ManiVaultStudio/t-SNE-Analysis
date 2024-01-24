#include "GradientDescentSettingsAction.h"

#include "TsneParameters.h"

using namespace mv::gui;

GradientDescentSettingsAction::GradientDescentSettingsAction(QObject* parent, TsneParameters& tsneParameters) :
    GroupAction(parent, "Gradient Descent Settings"),
    _tsneParameters(tsneParameters),
    _exaggerationAction(this, "Exaggeration"),
    _exponentialDecayAction(this, "Exponential decay")
{
    addAction(&_exaggerationAction);
    addAction(&_exponentialDecayAction);

    _exaggerationAction.setDefaultWidgetFlags(IntegralAction::SpinBox);
    _exponentialDecayAction.setDefaultWidgetFlags(IntegralAction::SpinBox);

    _exaggerationAction.initialize(1, 10000, 250);
    _exponentialDecayAction.initialize(1, 10000, 70);

    const auto updateExaggeration = [this]() -> void {
        _tsneParameters.setExaggerationIter(_exaggerationAction.getValue());
    };

    const auto updateExponentialDecay = [this]() -> void {
        _tsneParameters.setExponentialDecayIter(_exponentialDecayAction.getValue());
    };

    const auto isResettable = [this]() -> bool {
        if (_exaggerationAction.isResettable())
            return true;

        if (_exponentialDecayAction.isResettable())
            return true;

        return false;
    };

    const auto updateReadOnly = [this, isResettable]() -> void {
        const auto enable = !isReadOnly();

        _exaggerationAction.setEnabled(enable);
        _exponentialDecayAction.setEnabled(enable);
    };

    connect(&_exaggerationAction, &IntegralAction::valueChanged, this, [this, updateExaggeration](const std::int32_t& value) {
        updateExaggeration();
    });

    connect(&_exponentialDecayAction, &IntegralAction::valueChanged, this, [this, updateExponentialDecay](const std::int32_t& value) {
        updateExponentialDecay();
    });

    connect(this, &GroupAction::readOnlyChanged, this, [this, updateReadOnly](const bool& readOnly) {
        updateReadOnly();
    });

    updateExaggeration();
    updateExponentialDecay();
    updateReadOnly();
}

void GradientDescentSettingsAction::fromVariantMap(const QVariantMap& variantMap)
{
    GroupAction::fromVariantMap(variantMap);

    _exaggerationAction.fromParentVariantMap(variantMap);
    _exponentialDecayAction.fromParentVariantMap(variantMap);
}

QVariantMap GradientDescentSettingsAction::toVariantMap() const
{
    QVariantMap variantMap = GroupAction::toVariantMap();

    _exaggerationAction.insertIntoVariantMap(variantMap);
    _exponentialDecayAction.insertIntoVariantMap(variantMap);

    return variantMap;
}
