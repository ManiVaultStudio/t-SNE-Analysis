#pragma once

#include "actions/GroupAction.h"
#include "actions/IntegralAction.h"

using namespace mv::gui;

class TsneParameters;

/**
 * Gradient Descent setting action class
 *
 * Action class for Gradient Descent settings
 *
 * @author Thomas Kroes
 */
class GradientDescentSettingsAction : public GroupAction
{
public:

    /**
     * Constructor
     * @param tsneSettingsAction Reference to TSNE settings action
     */
    GradientDescentSettingsAction(QObject* parent, TsneParameters& tsneParameters);

public: // Action getters
    
    IntegralAction& getExaggerationAction() { return _exaggerationAction; };
    IntegralAction& getExponentialDecayAction() { return _exponentialDecayAction; };

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
    TsneParameters&         _tsneParameters;            /** Reference to tSNE parameters */
    IntegralAction          _exaggerationAction;        /** Exaggeration action */
    IntegralAction          _exponentialDecayAction;    /** Exponential decay action */
};
