#include "TsneSettingsAction.h"
#include "Application.h"

#include <QTabWidget>

using namespace hdps::gui;

TsneSettingsAction::TsneSettingsAction(QObject* parent) :
    WidgetAction(parent),
    _tsneParameters(),
    _computationAction(*this),
    _generalTsneSettingsAction(*this),
    _advancedTsneSettingsAction(*this)
{
    setText("TSNE");
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

TsneSettingsAction::Widget::Widget(QWidget* parent, TsneSettingsAction* tsneSettingsAction, const Widget::State& state) :
    WidgetAction::Widget(parent, tsneSettingsAction, state)
{
}