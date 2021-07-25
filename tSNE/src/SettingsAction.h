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
	TsneAnalysisPlugin*			_tsneAnalysisPlugin;		/** Pointer to TSNE analysis plugin */
	hdps::gui::OptionAction		_knnTypeAction;				/** KNN action */
	hdps::gui::OptionAction		_distanceMetricAction;		/** Distance metric action */
	hdps::gui::IntegralAction	_numIterationsAction;		/** Number of iterations action */
	hdps::gui::IntegralAction	_perplexityAction;			/** Perplexity action */
	hdps::gui::IntegralAction	_exaggerationAction;		/** Exaggeration action */
	hdps::gui::IntegralAction	_exponentialDecayAction;	/** Exponential decay action */
	hdps::gui::IntegralAction	_numTreesAction;			/** Exponential decay action */
	hdps::gui::IntegralAction	_numChecksAction;			/** Exponential decay action */
	hdps::gui::TriggerAction	_resetAction;				/** Reset all input to defaults */
	hdps::gui::TriggerAction	_startComputationAction;	/** Start computation action */
	hdps::gui::TriggerAction	_stopComputationAction;		/** Stop computation action */
	
    friend class Widget;
};