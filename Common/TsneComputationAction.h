#pragma once

#include "actions/Actions.h"

class TsneSettingsAction;
class QMenu;

/**
 * TSNE computation action class
 *
 * Actions class for starting/continuing/stopping the TSNE computation
 *
 * @author Thomas Kroes
 */
class TsneComputationAction : public hdps::gui::WidgetAction
{
protected:

    /** Widget class for TSNE computation action */
    class Widget : public hdps::gui::WidgetActionWidget {
    public:

        /**
         * Constructor
         * @param parent Pointer to parent widget
         * @param tsneComputationAction Pointer to TSNE computation action
         * @param state State of the widget
         */
        Widget(QWidget* parent, TsneComputationAction* tsneComputationAction, const hdps::gui::WidgetActionWidget::State& state);
    };

    /**
     * Get widget representation of the TSNE computation action
     * @param parent Pointer to parent widget
     * @param state Widget state
     */
    QWidget* getWidget(QWidget* parent, const hdps::gui::WidgetActionWidget::State& state = hdps::gui::WidgetActionWidget::State::Standard) override {
        return new Widget(parent, this, state);
    };

public:

    /**
     * Constructor
     * @param tsneSettingsAction Reference to TSNE settings action
     */
    TsneComputationAction(TsneSettingsAction& tsneSettingsAction);

    /**
     * Get the context menu for the action
     * @param parent Parent widget
     * @return Context menu
     */
    QMenu* getContextMenu(QWidget* parent = nullptr) override;

public: // Action getters

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