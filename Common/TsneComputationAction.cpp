#include "TsneComputationAction.h"
#include "TsneSettingsAction.h"

using namespace hdps::gui;

TsneComputationAction::TsneComputationAction(TsneSettingsAction& tsneSettingsAction) :
    WidgetAction(&tsneSettingsAction),
    _tsneSettingsAction(tsneSettingsAction),
    _startComputationAction(this, "Start"),
    _continueComputationAction(this, "Continue"),
    _stopComputationAction(this, "Stop"),
    _runningAction(this, "Running")
{
    setText("");

    _startComputationAction.setToolTip("Start the tSNE computation");
    _continueComputationAction.setToolTip("Continue with the tSNE computation");
    _stopComputationAction.setToolTip("Stop the current tSNE computation");
}

QMenu* TsneComputationAction::getContextMenu()
{
    auto menu = new QMenu(text());

    menu->addAction(&_startComputationAction);
    menu->addAction(&_continueComputationAction);
    menu->addAction(&_runningAction);

    return menu;
}

TsneComputationAction::Widget::Widget(QWidget* parent, TsneComputationAction* tsneComputationAction, const Widget::State& state) :
    WidgetAction::Widget(parent, tsneComputationAction, state)
{
    auto& tsneSettingsAction = tsneComputationAction->getTsneSettingsAction();

    auto layout = new QHBoxLayout();

    layout->setMargin(0);

    layout->addWidget(tsneComputationAction->getStartComputationAction().createWidget(this));
    layout->addWidget(tsneComputationAction->getContinueComputationAction().createWidget(this));
    layout->addWidget(tsneComputationAction->getStopComputationAction().createWidget(this));

    setLayout(layout);
}
