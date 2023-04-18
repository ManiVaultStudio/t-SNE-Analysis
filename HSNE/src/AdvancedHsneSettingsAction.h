#pragma once

#include "actions/Actions.h"

using namespace hdps::gui;

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

    friend class Widget;
};
