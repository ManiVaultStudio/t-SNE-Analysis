#pragma once

#include "actions/Actions.h"

class QMenu;
class HsneSettingsAction;

/**
 * General HSNE setting action class
 *
 * Actions class for general HSNE settings
 *
 * @author Thomas Kroes
 */
class GeneralHsneSettingsAction : public hdps::gui::WidgetActionGroup
{
protected:

    /** Widget class for general HSNE settings action */
    class Widget : public hdps::gui::WidgetActionGroup::FormWidget {
    public:

        /**
         * Constructor
         * @param parent Pointer to parent widget
         * @param generalHsneSettingsAction Pointer to general HSNE settings action
         * @param state State of the widget
         */
        Widget(QWidget* parent, GeneralHsneSettingsAction* generalHsneSettingsAction, const Widget::State& state);
    };

    /**
     * Get widget representation of the general HSNE settings action
     * @param parent Pointer to parent widget
     * @param state Widget state
     */
    QWidget* getWidget(QWidget* parent, const Widget::State& state = Widget::State::Standard) override {
        return new Widget(parent, this, state);
    };

public:

    /**
     * Constructor
     * @param hsneSettingsAction Reference to HSNE settings action
     */
    GeneralHsneSettingsAction(HsneSettingsAction& hsneSettingsAction);

public: // Action getters

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
