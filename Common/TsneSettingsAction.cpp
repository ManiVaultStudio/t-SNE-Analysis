#include "TsneSettingsAction.h"
#include "Application.h"

#include <QTabWidget>

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

QMenu* TsneSettingsAction::getContextMenu()
{
    auto menu = new QMenu(text());

    menu->addAction(&_computationAction.getStartComputationAction());
    menu->addAction(&_computationAction.getContinueComputationAction());

    menu->addSeparator();

    menu->addAction(&_computationAction.getStopComputationAction());

    return menu;
}
