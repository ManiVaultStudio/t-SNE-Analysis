#pragma once

#include "actions/Actions.h"

using namespace hdps::gui;

class QMenu;
class HsneSettingsAction;

namespace hdi {
    namespace dr {
        enum knn_distance_metric;
    }
}

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
    IntegralAction& getSeedAction() { return _seedAction; }
    ToggleAction& getUseMonteCarloSamplingAction() { return _useMonteCarloSamplingAction; }
    TriggerAction& getStartAction() { return _startAction; }

protected:
    HsneSettingsAction&     _hsneSettingsAction;                /** Reference to HSNE settings action */
    OptionAction            _knnTypeAction;                     /** KNN action */
    IntegralAction          _numScalesAction;                   /** Num scales action */
    IntegralAction          _seedAction;                        /** Random seed action */
    ToggleAction            _useMonteCarloSamplingAction;       /** Use Monte Carlo sampling on/off action */
    TriggerAction           _startAction;                       /** Start action */

    friend class Widget;
};
