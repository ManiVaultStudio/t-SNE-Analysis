#include "TsneComputationAction.h"
#include "TsneSettingsAction.h"

#include <QHBoxLayout>

using namespace hdps::gui;

TsneComputationAction::TsneComputationAction(QObject* parent) :
    WidgetAction(parent),
    _startComputationAction(this, "Start"),
    _continueComputationAction(this, "Continue"),
    _stopComputationAction(this, "Stop"),
    _runningAction(this, "Running")
{
    setText("Computation");

    _startComputationAction.setToolTip("Start the tSNE computation");
    _continueComputationAction.setToolTip("Continue with the tSNE computation");
    _stopComputationAction.setToolTip("Stop the current tSNE computation");
}

QMenu* TsneComputationAction::getContextMenu(QWidget* parent /*= nullptr*/)
{
    auto menu = new QMenu(text(), parent);

    menu->addAction(&_startComputationAction);
    menu->addAction(&_continueComputationAction);
    menu->addAction(&_stopComputationAction);

    return menu;
}

TsneComputationAction::Widget::Widget(QWidget* parent, TsneComputationAction* tsneComputationAction) :
    WidgetActionWidget(parent, tsneComputationAction)
{
    auto layout = new QHBoxLayout();

    layout->setMargin(0);

    layout->addWidget(tsneComputationAction->getStartComputationAction().createWidget(this));
    layout->addWidget(tsneComputationAction->getContinueComputationAction().createWidget(this));
    layout->addWidget(tsneComputationAction->getStopComputationAction().createWidget(this));

    setLayout(layout);
}
