#pragma once

#include "actions/GroupAction.h"
#include "actions/IntegralAction.h"
#include "actions/OptionAction.h"
#include "actions/ToggleAction.h"
#include "actions/TriggerAction.h"

using namespace mv::gui;

class HsneSettingsAction;

/**
 * General HSNE setting action class
 *
 * Actions class for general HSNE settings
 *
 * @author Thomas Kroes
 */
class GeneralHsneSettingsAction : public GroupAction
{
public:

    /**
     * Constructor
     * @param hsneSettingsAction Reference to HSNE settings action
     */
    GeneralHsneSettingsAction(HsneSettingsAction& hsneSettingsAction);

public: // Action getters

    HsneSettingsAction& getHsneSettingsAction() { return _hsneSettingsAction; }
    OptionAction& getKnnAlgorithmAction() { return _knnAlgorithmAction; }
    IntegralAction& getNumKnnAction() { return _numKnnAction; };
    OptionAction& getDistanceMetricAction() { return _distanceMetricAction; }
    ToggleAction& getPublishLandmarkWeightAction() { return _publishLandmarkWeightAction; }
    IntegralAction& getNumScalesAction() { return _numScalesAction; }
    TriggerAction& getStartAction() { return _startAction; }

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
    HsneSettingsAction&     _hsneSettingsAction;                    /** Reference to HSNE settings action */
    IntegralAction          _numScalesAction;                       /** Num scales action */
    OptionAction            _knnAlgorithmAction;                    /** KNN algorithm action */
    OptionAction            _distanceMetricAction;                  /** Distance metric action */
    ToggleAction            _publishLandmarkWeightAction;           /** Whether to create a dataset that contains the landmark weights */
    IntegralAction          _numKnnAction;                          /** Number of Knn action */
    TriggerAction           _startAction;                           /** Start action */
};
