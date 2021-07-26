#pragma once

#include "actions/Actions.h"

class QMenu;
class TsneAnalysisPlugin;

class AdvancedSettingsAction : public hdps::gui::WidgetActionGroup
{
protected:

    class Widget : public hdps::gui::WidgetActionGroup::Widget {
    public:
        Widget(QWidget* parent, AdvancedSettingsAction* advancedSettingsAction, const Widget::State& state);
    };

    QWidget* getWidget(QWidget* parent, const Widget::State& state = Widget::State::Standard) override {
        return new Widget(parent, this, state);
    };

public:
	AdvancedSettingsAction(TsneAnalysisPlugin* tsneAnalysisPlugin);

    QMenu* getContextMenu();

protected:
	TsneAnalysisPlugin*			_tsneAnalysisPlugin;		/** Pointer to TSNE analysis plugin */
	hdps::gui::IntegralAction	_exaggerationAction;		/** Exaggeration action */
	hdps::gui::IntegralAction	_exponentialDecayAction;	/** Exponential decay action */
	hdps::gui::IntegralAction	_numTreesAction;			/** Exponential decay action */
	hdps::gui::IntegralAction	_numChecksAction;			/** Exponential decay action */
	hdps::gui::TriggerAction	_resetAction;				/** Reset all input to defaults */
	
    friend class Widget;
};