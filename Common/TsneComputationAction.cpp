#include "TsneComputationAction.h"
#include "TsneSettingsAction.h"

#include <QHBoxLayout>
#include <QMenu>

using namespace mv::gui;

TsneComputationAction::TsneComputationAction(QObject* parent) :
    VerticalGroupAction(parent, "Computation"),
    _startComputationAction(this, "Start"),
    _continueComputationAction(this, "Continue"),
    _stopComputationAction(this, "Stop"),
    _runningAction(this, "Running")
{
    setShowLabels(false);
    setDefaultWidgetFlag(GroupAction::NoMargins);

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

void TsneComputationAction::fromVariantMap(const QVariantMap& variantMap)
{
    GroupAction::fromVariantMap(variantMap);

    _startComputationAction.fromParentVariantMap(variantMap);
    _continueComputationAction.fromParentVariantMap(variantMap);
    _stopComputationAction.fromParentVariantMap(variantMap);
    _runningAction.fromParentVariantMap(variantMap);
}

QVariantMap TsneComputationAction::toVariantMap() const
{
    qDebug() << "TsneComputationAction::toVariantMap:";

    QVariantMap variantMap = GroupAction::toVariantMap();

    _startComputationAction.insertIntoVariantMap(variantMap);
    _continueComputationAction.insertIntoVariantMap(variantMap);
    _stopComputationAction.insertIntoVariantMap(variantMap);
    _runningAction.insertIntoVariantMap(variantMap);

    return variantMap;
}
