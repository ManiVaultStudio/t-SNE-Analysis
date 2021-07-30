#pragma once

#include "actions/Actions.h"

class QMenu;
class TsneSettingsAction;

class GeneralTsneSettingsAction : public hdps::gui::WidgetActionGroup
{
protected:

    class Widget : public hdps::gui::WidgetActionGroup::Widget {
    public:
        Widget(QWidget* parent, GeneralTsneSettingsAction* generalSettingsAction, const Widget::State& state);
    };

    QWidget* getWidget(QWidget* parent, const Widget::State& state = Widget::State::Standard) override {
        return new Widget(parent, this, state);
    };

public:
    GeneralTsneSettingsAction(TsneSettingsAction& tsneSettingsAction);

    QMenu* getContextMenu() { return nullptr; }

    TsneSettingsAction& getTsneSettingsAction() { return _tsneSettingsAction; };
    hdps::gui::OptionAction& getKnnTypeAction() { return _knnTypeAction; };
    hdps::gui::OptionAction& getDistanceMetricAction() { return _distanceMetricAction; };
    hdps::gui::IntegralAction& getNumIterationsAction() { return _numIterationsAction; };
    hdps::gui::IntegralAction& getPerplexityAction() { return _perplexityAction; };
    hdps::gui::TriggerAction& getResetAction() { return _resetAction; };

protected:
    TsneSettingsAction&         _tsneSettingsAction;            /** Pointer to parent tSNE settings action */
    hdps::gui::OptionAction		_knnTypeAction;					/** KNN action */
    hdps::gui::OptionAction		_distanceMetricAction;			/** Distance metric action */
    hdps::gui::IntegralAction	_numIterationsAction;			/** Number of iterations action */
    hdps::gui::IntegralAction	_perplexityAction;				/** Perplexity action */
    hdps::gui::TriggerAction	_resetAction;					/** Reset all input to defaults */

    friend class Widget;
};