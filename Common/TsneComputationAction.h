#pragma once

#include "actions/Actions.h"

using namespace hdps::gui;

class TsneSettingsAction;
class QMenu;

/**
 * TSNE computation action class
 *
 * Actions class for starting/continuing/stopping the TSNE computation
 *
 * @author Thomas Kroes
 */
class TsneComputationAction : public WidgetAction
{
protected:

    /** Widget class for TSNE computation action */
    class Widget : public WidgetActionWidget {
    public:

        /**
         * Constructor
         * @param parent Pointer to parent widget
         * @param tsneComputationAction Pointer to TSNE computation action
         * @param state State of the widget
         */
        Widget(QWidget* parent, TsneComputationAction* tsneComputationAction, const WidgetActionWidget::State& state);
    };

    /**
     * Get widget representation of the TSNE computation action
     * @param parent Pointer to parent widget
     * @param widgetFlags Widget flags for the configuration of the widget (type)
     * @param state State of the widget (for stateful widgets)
     */
    QWidget* getWidget(QWidget* parent, const std::int32_t& widgetFlags, const WidgetActionWidget::State& state = WidgetActionWidget::State::Standard) override {
        return new Widget(parent, this, state);
    };

public:

    /**
     * Constructor
     * @param parent Pointer to parent object
     */
    TsneComputationAction(QObject* parent);

    /**
     * Get the context menu for the action
     * @param parent Parent widget
     * @return Context menu
     */
    QMenu* getContextMenu(QWidget* parent = nullptr) override;

public: // Action getters

    TriggerAction& getStartComputationAction() { return _startComputationAction; }
    TriggerAction& getContinueComputationAction() { return _continueComputationAction; }
    TriggerAction& getStopComputationAction() { return _stopComputationAction; }
    ToggleAction& getRunningAction() { return _runningAction; }

protected:
    TriggerAction           _startComputationAction;        /** Start computation action */
    TriggerAction           _continueComputationAction;     /** Continue computation action */
    TriggerAction           _stopComputationAction;         /** Stop computation action */
    ToggleAction            _runningAction;                 /** Running action */

    friend class Widget;
};