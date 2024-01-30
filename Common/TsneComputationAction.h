#pragma once

#include <actions/GroupAction.h>
#include <actions/IntegralAction.h>
#include <actions/ToggleAction.h>
#include <actions/TriggerAction.h>

using namespace mv::gui;

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
public:

    /**
     * Constructor
     * @param parent Pointer to parent object
     */
    TsneComputationAction(GroupAction* parent);

    /**
     * Get the context menu for the action
     * @param parent Parent widget
     * @return Context menu
     */
    QMenu* getContextMenu(QWidget* parent = nullptr) override;

    /**
     * Add member action to parent
     */
    void addActions();

public: // Action getters

    IntegralAction& getNumIterationsAction() { return _numIterationsAction; };
    IntegralAction& getNumberOfComputatedIterationsAction() { return _numberOfComputatedIterationsAction; };
    IntegralAction& getUpdateIterationsAction() { return _updateIterationsAction; };
    TriggerAction& getStartComputationAction() { return _startComputationAction; }
    TriggerAction& getContinueComputationAction() { return _continueComputationAction; }
    TriggerAction& getStopComputationAction() { return _stopComputationAction; }
    ToggleAction& getRunningAction() { return _runningAction; }

public: // Serialization

    /**
     * Load plugin from variant map
     * @param Variant map representation of the plugin
     */
    void fromVariantMap(const QVariantMap& variantMap) override;

    /**
     * Save plugin to variant map
     * @return Variant map representation of the plugin
     */
    QVariantMap toVariantMap() const override;

private:
    IntegralAction          _numIterationsAction;                   /** Number of iterations action */
    IntegralAction          _numberOfComputatedIterationsAction;    /** Number of computed iterations action */
    IntegralAction          _updateIterationsAction;                /** Number of update iterations (copying embedding to ManiVault core) */

    TriggerAction           _startComputationAction;                /** Start computation action */
    TriggerAction           _continueComputationAction;             /** Continue computation action */
    TriggerAction           _stopComputationAction;                 /** Stop computation action */

    ToggleAction            _runningAction;                         /** Running action */
};