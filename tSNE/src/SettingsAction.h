#pragma once

#include "actions/Actions.h"

class QMenu;
class TsneAnalysisPlugin;

class SettingsAction : public hdps::gui::WidgetAction
{
protected:

    class Widget : public hdps::gui::WidgetAction::Widget {
    public:
        Widget(QWidget* parent, SettingsAction* settingsAction, const Widget::State& state);
    };

    QWidget* getWidget(QWidget* parent, const Widget::State& state = Widget::State::Standard) override {
        return new Widget(parent, this, state);
    };

public:
	SettingsAction(TsneAnalysisPlugin* tsneAnalysisPlugin);

    QMenu* getContextMenu();

	hdps::gui::TriggerAction& getStartComputationAction() { return _startComputationAction; }
	hdps::gui::TriggerAction& getStopComputationAction() { return _stopComputationAction; }

protected:
	hdps::gui::TriggerAction	_startComputationAction;	/** Start computation action */
	hdps::gui::TriggerAction	_stopComputationAction;		/** Stop computation action */

    friend class Widget;
};