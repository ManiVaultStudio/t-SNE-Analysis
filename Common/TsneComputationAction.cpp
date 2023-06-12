#include "TsneComputationAction.h"
#include "TsneSettingsAction.h"

#include <QHBoxLayout>
#include <QMenu>

using namespace hdps::gui;

TsneComputationAction::TsneComputationAction(QObject* parent) :
    HorizontalGroupAction(parent, "Group"),
    _startComputationAction(this, "Start"),
    _continueComputationAction(this, "Continue"),
    _stopComputationAction(this, "Stop"),
    _runningAction(this, "Running")
{
    setText("Computation");

    addAction(&_startComputationAction);
    addAction(&_continueComputationAction);
    addAction(&_stopComputationAction);
    addAction(&_runningAction);

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