#pragma once

#include "actions/GroupAction.h"
#include "actions/IntegralAction.h"
#include "actions/OptionAction.h"
#include "actions/ToggleAction.h"
#include "actions/TriggerAction.h"

#include "hdi/dimensionality_reduction/knn_utils.h"

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

public slots:
    void setPerplexity(const int32_t& perplexity);
    void setDistanceMetric(const hdi::dr::knn_distance_metric& metric);

public: // Action getters

    HsneSettingsAction& getHsneSettingsAction() { return _hsneSettingsAction; }
    OptionAction& getKnnTypeAction() { return _knnTypeAction; }
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
    HsneSettingsAction&     _hsneSettingsAction;                /** Reference to HSNE settings action */
    OptionAction            _knnTypeAction;                     /** KNN action */
    IntegralAction          _numScalesAction;                   /** Num scales action */
    TriggerAction           _startAction;                       /** Start action */

    friend class Widget;
};
