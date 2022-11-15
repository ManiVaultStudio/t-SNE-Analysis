#include "TsneSettingsAction.h"

#include <QMenu>

using namespace hdps::gui;

TsneSettingsAction::TsneSettingsAction(QObject* parent) :
    GroupAction(parent, false),
    _tsneParameters(),
    _generalTsneSettingsAction(*this),
    _advancedTsneSettingsAction(*this)
{
    setText("TSNE");

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
