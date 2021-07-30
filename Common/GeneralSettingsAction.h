#pragma once

#include "actions/Actions.h"

class QMenu;
class TsneAnalysisPlugin;

class GeneralSettingsAction : public hdps::gui::WidgetActionGroup
{
protected:

    class Widget : public hdps::gui::WidgetActionGroup::Widget {
    public:
        Widget(QWidget* parent, GeneralSettingsAction* generalSettingsAction, const Widget::State& state);
    };

    QWidget* getWidget(QWidget* parent, const Widget::State& state = Widget::State::Standard) override {
        return new Widget(parent, this, state);
    };

public:
    GeneralSettingsAction(TsneAnalysisPlugin* tsneAnalysisPlugin);

    QMenu* getContextMenu() { return nullptr; }

    hdps::gui::TriggerAction& getStartComputationAction() { return _startComputationAction; }
    hdps::gui::TriggerAction& getContinueComputationAction() { return _continueComputationAction; }
    hdps::gui::TriggerAction& getStopComputationAction() { return _stopComputationAction; }
    hdps::gui::ToggleAction& getRunningAction() { return _runningAction; }

protected:
    TsneAnalysisPlugin*			_tsneAnalysisPlugin;			/** Pointer to TSNE analysis plugin */
    hdps::gui::OptionAction		_knnTypeAction;					/** KNN action */
    hdps::gui::OptionAction		_distanceMetricAction;			/** Distance metric action */
    hdps::gui::IntegralAction	_numIterationsAction;			/** Number of iterations action */
    hdps::gui::IntegralAction	_perplexityAction;				/** Perplexity action */
    hdps::gui::TriggerAction	_resetAction;					/** Reset all input to defaults */
    hdps::gui::TriggerAction	_startComputationAction;		/** Start computation action */
    hdps::gui::TriggerAction	_continueComputationAction;		/** Start computation action */
    hdps::gui::TriggerAction	_stopComputationAction;			/** Stop computation action */
    hdps::gui::ToggleAction		_runningAction;					/** Running action */

    friend class Widget;
};