#include "KnnSettingsAction.h"

#include "KnnParameters.h"

using namespace mv::gui;

KnnSettingsAction::KnnSettingsAction(QObject* parent, KnnParameters& knnParameters) :
    GroupAction(parent, "Knn Settings"),
    _knnParameters(knnParameters),
    _numTreesAction(this, "Annoy Trees"),
    _numChecksAction(this, "Annoy Checks"),
    _mAction(this, "HNSW M"),
    _efAction(this, "HNSW ef")
{
    addAction(&_numTreesAction);
    addAction(&_numChecksAction);
    addAction(&_mAction);
    addAction(&_efAction);

    _numTreesAction.setDefaultWidgetFlags(IntegralAction::SpinBox);
    _numChecksAction.setDefaultWidgetFlags(IntegralAction::SpinBox);
    _mAction.setDefaultWidgetFlags(IntegralAction::SpinBox);
    _efAction.setDefaultWidgetFlags(IntegralAction::SpinBox);

    _numTreesAction.initialize(1, 10000, 4);
    _numChecksAction.initialize(1, 10000, 1024);
    _mAction.initialize(2, 300, 16);
    _efAction.initialize(1, 10000, 200);

    const auto updateNumTrees = [this]() -> void {
        _knnParameters.setAnnoyNumTrees(_numTreesAction.getValue());
    };

    const auto updateNumChecks = [this]() -> void {
        _knnParameters.setAnnoyNumChecks(_numChecksAction.getValue());
    };

    const auto updateM = [this]() -> void {
        _knnParameters.setHNSWm(_mAction.getValue());
    };

    const auto updateEf = [this]() -> void {
        _knnParameters.setHNSWef(_efAction.getValue());
    };

    const auto updateReadOnly = [this]() -> void {
        const auto enable = !isReadOnly();

        _numTreesAction.setEnabled(enable);
        _numChecksAction.setEnabled(enable);
        _mAction.setEnabled(enable);
        _efAction.setEnabled(enable);
    };

    connect(&_numTreesAction, &IntegralAction::valueChanged, this, [this, updateNumTrees](const std::int32_t& value) {
        updateNumTrees();
    });

    connect(&_numChecksAction, &IntegralAction::valueChanged, this, [this, updateNumChecks](const std::int32_t& value) {
        updateNumChecks();
    });

    connect(&_mAction, &IntegralAction::valueChanged, this, [this, updateM](const std::int32_t& value) {
        updateM();
    });

    connect(&_efAction, &IntegralAction::valueChanged, this, [this, updateEf](const std::int32_t& value) {
        updateEf();
    });

    connect(this, &GroupAction::readOnlyChanged, this, [this, updateReadOnly](const bool& readOnly) {
        updateReadOnly();
    });

    updateNumTrees();
    updateNumChecks();
    updateM();
    updateEf();
    updateReadOnly();
}

void KnnSettingsAction::fromVariantMap(const QVariantMap& variantMap)
{
    GroupAction::fromVariantMap(variantMap);

    _numTreesAction.fromParentVariantMap(variantMap);
    _numChecksAction.fromParentVariantMap(variantMap);
    _mAction.fromParentVariantMap(variantMap);
    _efAction.fromParentVariantMap(variantMap);
}

QVariantMap KnnSettingsAction::toVariantMap() const
{
    QVariantMap variantMap = GroupAction::toVariantMap();

    _numTreesAction.insertIntoVariantMap(variantMap);
    _numChecksAction.insertIntoVariantMap(variantMap);
    _mAction.insertIntoVariantMap(variantMap);
    _efAction.insertIntoVariantMap(variantMap);

    return variantMap;
}
