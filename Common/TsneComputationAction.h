#pragma once

#include <actions/HorizontalGroupAction.h>
#include <actions/TriggerAction.h>
#include <actions/ToggleAction.h>

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
class TsneComputationAction : public HorizontalGroupAction
{
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
    TriggerAction   _startComputationAction;        /** Start computation action */
    TriggerAction   _continueComputationAction;     /** Continue computation action */
    TriggerAction   _stopComputationAction;         /** Stop computation action */
    ToggleAction    _runningAction;                 /** Running action */

    friend class Widget;
};