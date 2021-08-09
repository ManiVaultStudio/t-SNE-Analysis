#include "TsneSettingsAction.h"

using namespace hdps::gui;

TsneSettingsAction::TsneSettingsAction(QObject* parent) :
    WidgetActionGroup(parent),
    _tsneParameters(),
    _computationAction(*this),
    _generalTsneSettingsAction(*this),
    _advancedTsneSettingsAction(*this)
{
    setText("TSNE");

    const auto updateReadOnly = [this]() -> void {
        _generalTsneSettingsAction.setReadOnly(isReadOnly());
        _advancedTsneSettingsAction.setReadOnly(isReadOnly());
    };

    connect(this, &WidgetActionGroup::readOnlyChanged, this, [this, updateReadOnly](const bool& readOnly) {
        updateReadOnly();
    });

    updateReadOnly();
}

QMenu* TsneSettingsAction::getContextMenu(QWidget* parent /*= nullptr*/)
{
    auto menu = new QMenu(text(), parent);

    menu->addAction(&_computationAction.getStartComputationAction());
    menu->addAction(&_computationAction.getContinueComputationAction());
    menu->addAction(&_computationAction.getStopComputationAction());

    return menu;
}
