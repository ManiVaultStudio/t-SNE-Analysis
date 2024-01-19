#include "TsneSettingsAction.h"

#include <QMenu>

using namespace mv::gui;

TsneSettingsAction::TsneSettingsAction(QObject* parent) :
    GroupAction(parent, "TSNE Settings"),
    _tsneParameters(),
    _generalTsneSettingsAction(*this),
    _advancedTsneSettingsAction(*this)
{
    const auto updateReadOnly = [this]() -> void {
        _generalTsneSettingsAction.setReadOnly(isReadOnly());
        _advancedTsneSettingsAction.setReadOnly(isReadOnly());
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
    _advancedTsneSettingsAction.fromParentVariantMap(variantMap);
}

QVariantMap TsneSettingsAction::toVariantMap() const
{
    QVariantMap variantMap = GroupAction::toVariantMap();

    _generalTsneSettingsAction.insertIntoVariantMap(variantMap);
    _advancedTsneSettingsAction.insertIntoVariantMap(variantMap);

    return variantMap;
}
