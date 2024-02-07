#include "TsneComputationAction.h"

#include "TsneParameters.h"

#include <actions/VerticalGroupAction.h>

#include <QMenu>

using namespace mv::gui;

TsneComputationAction::TsneComputationAction(GroupAction* parent, TsneParameters* tsneParameters) :
    WidgetAction(parent, "TsneComputationAction"),
    _numIterationsAction(this, "New iterations", 1, 10000, 1000),
    _numberOfComputatedIterationsAction(this, "Computed iterations", 0, std::numeric_limits<int>::max(), 0),
    _updateIterationsAction(this, "Core update every", 0, 10000, 10),
    _startComputationAction(this, "Start"),
    _continueComputationAction(this, "Continue"),
    _stopComputationAction(this, "Stop"),
    _runningAction(this, "Running"),
    _tsneParameters(tsneParameters)
{
    _numIterationsAction.setDefaultWidgetFlags(IntegralAction::SpinBox);
    _numberOfComputatedIterationsAction.setDefaultWidgetFlags(IntegralAction::LineEdit);
    _updateIterationsAction.setDefaultWidgetFlags(IntegralAction::SpinBox | IntegralAction::Slider);

    _updateIterationsAction.setToolTip("Update the dataset every x iterations. If set to 0, there will be no intermediate result.");
    _numIterationsAction.setToolTip("Number of new iterations that will be computed when pressing start or continue.");
    _numberOfComputatedIterationsAction.setToolTip("Number of iterations that have already been computed.");
    _startComputationAction.setToolTip("Start the tSNE computation");
    _continueComputationAction.setToolTip("Continue with the tSNE computation");
    _stopComputationAction.setToolTip("Stop the current tSNE computation");

    _numberOfComputatedIterationsAction.setEnabled(false);

    if (_tsneParameters)
    {
        const auto updateNumIterations = [this]() -> void {
            _tsneParameters->setExaggerationIter(_numIterationsAction.getValue());
            };

        connect(&_numIterationsAction, &IntegralAction::valueChanged, this, [this, updateNumIterations](int32_t val) {
            updateNumIterations();
            });

        const auto updateUpdateIterations = [this]() -> void {
            _tsneParameters->setUpdateCore(_updateIterationsAction.getValue());
            };

        connect(&_updateIterationsAction, &IntegralAction::valueChanged, this, [this, updateUpdateIterations](int32_t val) {
            updateUpdateIterations();
            });

        updateNumIterations();
        updateUpdateIterations();
    }
}

void TsneComputationAction::setReadOnly(bool readonly)
{
    _numIterationsAction.setEnabled(readonly);
    _updateIterationsAction.setEnabled(readonly);
    _startComputationAction.setEnabled(readonly);
    _continueComputationAction.setEnabled(readonly);
    _stopComputationAction.setEnabled(readonly);
}

void TsneComputationAction::addActions() 
{
    auto buttonGroup = new VerticalGroupAction(parent(), "Computation");
    buttonGroup->setShowLabels(false);
    buttonGroup->setDefaultWidgetFlag(GroupAction::NoMargins);

    auto parentAction = dynamic_cast<GroupAction*>(parent());

    parentAction->addAction(&_numIterationsAction);
    parentAction->addAction(&_numberOfComputatedIterationsAction);

    buttonGroup->addAction(&_startComputationAction);
    buttonGroup->addAction(&_continueComputationAction);
    buttonGroup->addAction(&_stopComputationAction);

    parentAction->addAction(buttonGroup);
}

QMenu* TsneComputationAction::getContextMenu(QWidget* parent /*= nullptr*/)
{
    auto menu = new QMenu("t-SNE", parent);

    menu->addAction(&_startComputationAction);
    menu->addAction(&_continueComputationAction);
    menu->addAction(&_stopComputationAction);

    return menu;
}

void TsneComputationAction::fromVariantMap(const QVariantMap& variantMap)
{
    WidgetAction::fromVariantMap(variantMap);

    _numIterationsAction.fromParentVariantMap(variantMap);
    _numberOfComputatedIterationsAction.fromParentVariantMap(variantMap);
    _updateIterationsAction.fromParentVariantMap(variantMap);
    _startComputationAction.fromParentVariantMap(variantMap);
    _continueComputationAction.fromParentVariantMap(variantMap);
    _stopComputationAction.fromParentVariantMap(variantMap);
    _runningAction.fromParentVariantMap(variantMap);
}

QVariantMap TsneComputationAction::toVariantMap() const
{
    QVariantMap variantMap = WidgetAction::toVariantMap();

    _numIterationsAction.insertIntoVariantMap(variantMap);
    _numberOfComputatedIterationsAction.insertIntoVariantMap(variantMap);
    _updateIterationsAction.insertIntoVariantMap(variantMap);
    _startComputationAction.insertIntoVariantMap(variantMap);
    _continueComputationAction.insertIntoVariantMap(variantMap);
    _stopComputationAction.insertIntoVariantMap(variantMap);
    _runningAction.insertIntoVariantMap(variantMap);

    return variantMap;
}
