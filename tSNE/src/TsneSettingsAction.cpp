#include "TsneSettingsAction.h"

#include <QMenu>

using namespace mv::gui;

TsneSettingsAction::TsneSettingsAction(QObject* parent) :
    GroupAction(parent, "TSNE Settings"),
    _tsneParameters(),
    _knnParameters(),
    _generalTsneSettingsAction(*this),
    _gradientDescentSettingsAction(this, _tsneParameters),
    _knnSettingsAction(this, _knnParameters)
{
    const auto updateReadOnly = [this]() -> void {
        _generalTsneSettingsAction.setReadOnly(isReadOnly());
        _gradientDescentSettingsAction.setReadOnly(isReadOnly());
        _knnSettingsAction.setReadOnly(isReadOnly());
    };

    connect(this, &GroupAction::readOnlyChanged, this, [this, updateReadOnly](const bool& readOnly) {
        updateReadOnly();
    });

    updateReadOnly();
}

QMenu* TsneSettingsAction::getContextMenu(QWidget* parent /*= nullptr*/)
{
    auto menu = new QMenu(text(), parent);

    auto& computationAction = _generalTsneSettingsAction.getComputationAction();

    menu->addAction(&computationAction.getStartComputationAction());
    menu->addAction(&computationAction.getContinueComputationAction());
    menu->addAction(&computationAction.getStopComputationAction());

    return menu;
}

void TsneSettingsAction::fromVariantMap(const QVariantMap& variantMap)
{
    GroupAction::fromVariantMap(variantMap);

    _generalTsneSettingsAction.fromParentVariantMap(variantMap);
    _gradientDescentSettingsAction.fromVariantMap(variantMap["Gradient Descent Settings"].toMap());
    _knnSettingsAction.fromVariantMap(variantMap["Knn Settings"].toMap());
}

QVariantMap TsneSettingsAction::toVariantMap() const
{
    QVariantMap variantMap = GroupAction::toVariantMap();

    _generalTsneSettingsAction.insertIntoVariantMap(variantMap);
    _gradientDescentSettingsAction.insertIntoVariantMap(variantMap);
    _knnSettingsAction.insertIntoVariantMap(variantMap);

    return variantMap;
}
