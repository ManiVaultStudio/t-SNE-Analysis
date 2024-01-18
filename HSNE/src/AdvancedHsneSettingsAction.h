#pragma once

#include "actions/DecimalAction.h"
#include "actions/GroupAction.h"
#include "actions/IntegralAction.h"
#include "actions/ToggleAction.h"

using namespace mv::gui;

class QMenu;
class HsneSettingsAction;

/**
 * Advanced HSNE setting action class
 *
 * Action class for advanced HSNE settings
 *
 * @author Thomas Kroes
 */
class AdvancedHsneSettingsAction : public GroupAction
{
public:

    /**
     * Constructor
     * @param hsneSettingsAction Reference to HSNE settings action
     */
    AdvancedHsneSettingsAction(HsneSettingsAction& hsneSettingsAction);

public: // Action getters

    HsneSettingsAction& getHsneSettingsAction() { return _hsneSettingsAction; }
    IntegralAction& getNumWalksForLandmarkSelectionAction() { return _numWalksForLandmarkSelectionAction; }
    DecimalAction& getNumWalksForLandmarkSelectionThresholdAction() { return _numWalksForLandmarkSelectionThresholdAction; }
    IntegralAction& getRandomWalkLengthAction() { return _randomWalkLengthAction; }
    IntegralAction& getNumWalksForAreaOfInfluenceAction() { return _numWalksForAreaOfInfluenceAction; }
    IntegralAction& getMinWalksRequiredAction() { return _minWalksRequiredAction; }
    IntegralAction& getNumChecksAknnAction() { return _numChecksAknnAction; }
    ToggleAction& getUseOutOfCoreComputationAction() { return _useOutOfCoreComputationAction; }
    ToggleAction& getSaveHierarchyToDiskAction() { return _saveHierarchyToDiskAction; }
    ToggleAction& getSaveHierarchyToProjectAction() { return _saveHierarchyToProjectAction; }

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

protected:
    HsneSettingsAction&     _hsneSettingsAction;                                /** Reference to HSNE settings action */
    IntegralAction          _numWalksForLandmarkSelectionAction;                /** Number of walks for landmark selection action */
    DecimalAction           _numWalksForLandmarkSelectionThresholdAction;       /** Number of walks for landmark selection threshold action */
    IntegralAction          _randomWalkLengthAction;                            /** Random walk length action */
    IntegralAction          _numWalksForAreaOfInfluenceAction;                  /** Number of walks for area of influence action */
    IntegralAction          _minWalksRequiredAction;                            /** Minimum number of walks required action */
    IntegralAction          _numChecksAknnAction;                               /** Number of KNN checks action */
    ToggleAction            _useOutOfCoreComputationAction;                     /** Use out of core computation action */
    ToggleAction            _saveHierarchyToDiskAction;                         /** Save computed hierarchy to disk action */
    ToggleAction            _saveHierarchyToProjectAction;                      /** Save computed hierarchy to project action */

    friend class Widget;
};
