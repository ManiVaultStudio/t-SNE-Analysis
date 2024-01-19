#include "KnnSettingsAction.h"

#include "TsneParameters.h"

using namespace mv::gui;

KnnSettingsAction::KnnSettingsAction(QObject* parent, TsneParameters& tsneParameters) :
    GroupAction(parent, "Gradient Descent Settings"),
    _tsneParameters(tsneParameters),
    _numTreesAction(this, "Number of trees"),
    _numChecksAction(this, "Number of checks")
{
    setObjectName("Gradient Descent Settings");

    addAction(&_numTreesAction);
    addAction(&_numChecksAction);

    _numTreesAction.setDefaultWidgetFlags(IntegralAction::SpinBox);
    _numChecksAction.setDefaultWidgetFlags(IntegralAction::SpinBox);

    _numTreesAction.initialize(1, 10000, 4);
    _numChecksAction.initialize(1, 10000, 1024);


    const auto updateNumTrees = [this]() -> void {
        _tsneParameters.setNumTrees(_numTreesAction.getValue());
        };

    const auto updateNumChecks = [this]() -> void {
        _tsneParameters.setNumChecks(_numChecksAction.getValue());
        };

    const auto isResettable = [this]() -> bool {
        if (_numTreesAction.isResettable())
            return true;

        if (_numChecksAction.isResettable())
            return true;

        return false;
        };

    const auto updateReadOnly = [this, isResettable]() -> void {
        const auto enable = !isReadOnly();

        _numTreesAction.setEnabled(enable);
        _numChecksAction.setEnabled(enable);
        };

    connect(&_numTreesAction, &IntegralAction::valueChanged, this, [this, updateNumTrees](const std::int32_t& value) {
        updateNumTrees();
        });

    connect(&_numChecksAction, &IntegralAction::valueChanged, this, [this, updateNumChecks](const std::int32_t& value) {
        updateNumChecks();
        });

    connect(this, &GroupAction::readOnlyChanged, this, [this, updateReadOnly](const bool& readOnly) {
        updateReadOnly();
        });

    updateNumTrees();
    updateNumChecks();
    updateReadOnly();
}

void KnnSettingsAction::fromVariantMap(const QVariantMap& variantMap)
{
    GroupAction::fromVariantMap(variantMap);

    _numTreesAction.fromParentVariantMap(variantMap);
    _numChecksAction.fromParentVariantMap(variantMap);
}

QVariantMap KnnSettingsAction::toVariantMap() const
{
    QVariantMap variantMap = GroupAction::toVariantMap();

    _numTreesAction.insertIntoVariantMap(variantMap);
    _numChecksAction.insertIntoVariantMap(variantMap);

    return variantMap;
}
