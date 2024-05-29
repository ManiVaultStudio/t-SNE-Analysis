#pragma once

#include "actions/DecimalAction.h"
#include "actions/GroupAction.h"
#include "actions/IntegralAction.h"
#include "actions/OptionAction.h"

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
    
    DecimalAction& getExaggerationFactorAction() { return _exaggerationFactorAction; };
    IntegralAction& getExaggerationIterAction() { return _exaggerationIterAction; };
    IntegralAction& getExponentialDecayAction() { return _exponentialDecayAction; };
    OptionAction& getGradientDescentTypeAction() { return _gradientDescentTypeAction; };

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
    DecimalAction           _exaggerationFactorAction;  /** Exaggeration factor action */
    IntegralAction          _exaggerationIterAction;    /** Exaggeration iteration action */
    IntegralAction          _exponentialDecayAction;    /** Exponential decay action */
    OptionAction            _gradientDescentTypeAction; /** GPU or CPU gradient descent */
};
