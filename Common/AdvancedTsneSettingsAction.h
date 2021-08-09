#pragma once

#include "actions/Actions.h"

class QMenu;
class TsneSettingsAction;

/**
 * Advanced TSNE setting action class
 *
 * Action class for advanced TSNE settings
 *
 * @author Thomas Kroes
 */
class AdvancedTsneSettingsAction : public hdps::gui::WidgetActionGroup
{
protected:

    /** Widget class for advanced TSNE settings action */
    class Widget : public hdps::gui::WidgetActionGroup::GroupWidget {
    public:

        /**
         * Constructor
         * @param parent Pointer to parent widget
         * @param advancedSettingsAction Pointer to advanced TSNE settings action
         * @param state State of the widget
         */
        Widget(QWidget* parent, AdvancedTsneSettingsAction* advancedSettingsAction, const Widget::State& state);
    };

    /**
     * Get widget representation of the advanced TSNE settings action
     * @param parent Pointer to parent widget
     * @param state Widget state
     */
    QWidget* getWidget(QWidget* parent, const Widget::State& state = Widget::State::Standard) override {
        return new Widget(parent, this, state);
    };

public:

    /**
     * Constructor
     * @param tsneSettingsAction Reference to TSNE settings action
     */
    AdvancedTsneSettingsAction(TsneSettingsAction& tsneSettingsAction);

public: // Action getters
    
    TsneSettingsAction& getTsneSettingsAction() { return _tsneSettingsAction; };
    hdps::gui::IntegralAction& getExaggerationAction() { return _exaggerationAction; };
    hdps::gui::IntegralAction& getExponentialDecayAction() { return _exponentialDecayAction; };
    hdps::gui::IntegralAction& getNumTreesAction() { return _numTreesAction; };
    hdps::gui::IntegralAction& getNumChecksAction() { return _numChecksAction; };
    hdps::gui::TriggerAction& getResetAction() { return _resetAction; };

protected:
    TsneSettingsAction&         _tsneSettingsAction;        /** Pointer to parent tSNE settings action */
    hdps::gui::IntegralAction   _exaggerationAction;        /** Exaggeration action */
    hdps::gui::IntegralAction   _exponentialDecayAction;    /** Exponential decay action */
    hdps::gui::IntegralAction   _numTreesAction;            /** Exponential decay action */
    hdps::gui::IntegralAction   _numChecksAction;           /** Exponential decay action */
    hdps::gui::TriggerAction    _resetAction;               /** Reset all input to defaults */

    friend class Widget;
};
