#pragma once

#include "actions/Actions.h"

class QMenu;
class HsneSettingsAction;

class GeneralHsneSettingsAction : public hdps::gui::WidgetActionGroup
{
protected:

    class Widget : public hdps::gui::WidgetActionGroup::GroupWidget {
    public:
        Widget(QWidget* parent, GeneralHsneSettingsAction* generalHsneSettingsAction, const Widget::State& state);
    };

    QWidget* getWidget(QWidget* parent, const Widget::State& state = Widget::State::Standard) override {
        return new Widget(parent, this, state);
    };

public:
    GeneralHsneSettingsAction(HsneSettingsAction& hsneSettingsAction);

    QMenu* getContextMenu() { return nullptr; }

    HsneSettingsAction& getHsneSettingsAction() { return _hsneSettingsAction; }
    hdps::gui::OptionAction& getKnnTypeAction() { return _knnTypeAction; }
    hdps::gui::IntegralAction& getSeedAction() { return _seedAction; }
    hdps::gui::ToggleAction& getUseMonteCarloSamplingAction() { return _useMonteCarloSamplingAction; }

protected:
    HsneSettingsAction&         _hsneSettingsAction;                /** Reference to HSNE settings action */
    hdps::gui::OptionAction     _knnTypeAction;                     /** KNN action */
    hdps::gui::IntegralAction   _seedAction;                        /** Random seed action */
    hdps::gui::ToggleAction     _useMonteCarloSamplingAction;       /** Use Monte Carlo sampling on/off action */

    friend class Widget;
};
