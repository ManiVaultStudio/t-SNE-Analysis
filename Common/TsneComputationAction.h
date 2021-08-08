#pragma once

#include "actions/Actions.h"

class TsneSettingsAction;
class QMenu;

class TsneComputationAction : public hdps::gui::WidgetAction
{
protected:

    class Widget : public hdps::gui::WidgetAction::Widget {
    public:
        Widget(QWidget* parent, TsneComputationAction* tsneComputationAction, const Widget::State& state);
    };

    QWidget* getWidget(QWidget* parent, const Widget::State& state = Widget::State::Standard) override {
        return new Widget(parent, this, state);
    };

public:
    TsneComputationAction(TsneSettingsAction& tsneSettingsAction);

    QMenu* getContextMenu();

    TsneSettingsAction& getTsneSettingsAction() { return _tsneSettingsAction; }
    hdps::gui::TriggerAction& getStartComputationAction() { return _startComputationAction; }
    hdps::gui::TriggerAction& getContinueComputationAction() { return _continueComputationAction; }
    hdps::gui::TriggerAction& getStopComputationAction() { return _stopComputationAction; }
    hdps::gui::ToggleAction& getRunningAction() { return _runningAction; }

protected:
    TsneSettingsAction&         _tsneSettingsAction;            /** Reference to parent tSNE settings action */
    hdps::gui::TriggerAction    _startComputationAction;        /** Start computation action */
    hdps::gui::TriggerAction    _continueComputationAction;     /** Continue computation action */
    hdps::gui::TriggerAction    _stopComputationAction;         /** Stop computation action */
    hdps::gui::ToggleAction     _runningAction;                 /** Running action */

    friend class Widget;
};