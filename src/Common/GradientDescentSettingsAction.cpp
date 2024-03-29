#include "GradientDescentSettingsAction.h"

#include "TsneParameters.h"

using namespace mv::gui;

GradientDescentSettingsAction::GradientDescentSettingsAction(QObject* parent, TsneParameters& tsneParameters) :
    GroupAction(parent, "Gradient Descent Settings"),
    _tsneParameters(tsneParameters),
    _exaggerationFactorAction(this, "Exaggeration factor"),
    _exaggerationIterAction(this, "Exaggeration iterations"),
    _exponentialDecayAction(this, "Exponential decay")
{
    addAction(&_exaggerationFactorAction);
    addAction(&_exaggerationIterAction);
    addAction(&_exponentialDecayAction);

    _exaggerationFactorAction.setDefaultWidgetFlags(IntegralAction::SpinBox);
    _exaggerationIterAction.setDefaultWidgetFlags(IntegralAction::SpinBox);
    _exponentialDecayAction.setDefaultWidgetFlags(IntegralAction::SpinBox);

    _exaggerationFactorAction.initialize(0, 20, 4);
    _exaggerationIterAction.initialize(1, 10000, 250);
    _exponentialDecayAction.initialize(1, 10000, 70);

    _exaggerationFactorAction.setToolTip("Defaults to 4 + number of points / 60'000");
    _exponentialDecayAction.setToolTip("Iterations after 'Exaggeration iterations' during \nwhich the exaggeration factor exponentionally decays towards 1");

    const auto updateExaggerationFactor = [this]() -> void {
        _tsneParameters.setExaggerationFactor(_exaggerationFactorAction.getValue());
    };

    const auto updateExaggerationIter = [this]() -> void {
        _tsneParameters.setExaggerationIter(_exaggerationIterAction.getValue());
    };

    const auto updateExponentialDecay = [this]() -> void {
        _tsneParameters.setExponentialDecayIter(_exponentialDecayAction.getValue());
    };

    const auto updateReadOnly = [this]() -> void {
        const auto enable = !isReadOnly();

        _exaggerationFactorAction.setEnabled(enable);
        _exaggerationIterAction.setEnabled(enable);
        _exponentialDecayAction.setEnabled(enable);
    };

    connect(&_exaggerationFactorAction, &DecimalAction::valueChanged, this, [this, updateExaggerationFactor](const float value) {
        updateExaggerationFactor();
    });

    connect(&_exaggerationIterAction, &IntegralAction::valueChanged, this, [this, updateExaggerationIter](const std::int32_t& value) {
        updateExaggerationIter();
    });

    connect(&_exponentialDecayAction, &IntegralAction::valueChanged, this, [this, updateExponentialDecay](const std::int32_t& value) {
        updateExponentialDecay();
    });

    connect(this, &GroupAction::readOnlyChanged, this, [this, updateReadOnly](const bool& readOnly) {
        updateReadOnly();
    });

    updateExaggerationFactor();
    updateExaggerationIter();
    updateExponentialDecay();
    updateReadOnly();
}

void GradientDescentSettingsAction::fromVariantMap(const QVariantMap& variantMap)
{
    GroupAction::fromVariantMap(variantMap);

    _exaggerationFactorAction.fromParentVariantMap(variantMap);
    _exaggerationIterAction.fromParentVariantMap(variantMap);
    _exponentialDecayAction.fromParentVariantMap(variantMap);
}

QVariantMap GradientDescentSettingsAction::toVariantMap() const
{
    QVariantMap variantMap = GroupAction::toVariantMap();

    _exaggerationFactorAction.insertIntoVariantMap(variantMap);
    _exaggerationIterAction.insertIntoVariantMap(variantMap);
    _exponentialDecayAction.insertIntoVariantMap(variantMap);

    return variantMap;
}
