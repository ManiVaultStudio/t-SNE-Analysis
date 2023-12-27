#pragma once

#include "actions/Actions.h"

using namespace mv::gui;

class QMenu;
class TsneSettingsAction;

/**
 * Advanced TSNE setting action class
 *
 * Action class for advanced TSNE settings
 *
 * @author Thomas Kroes
 */
class AdvancedTsneSettingsAction : public GroupAction
{
public:

    /**
     * Constructor
     * @param tsneSettingsAction Reference to TSNE settings action
     */
    AdvancedTsneSettingsAction(TsneSettingsAction& tsneSettingsAction);

public: // Action getters
    
    TsneSettingsAction& getTsneSettingsAction() { return _tsneSettingsAction; };
    IntegralAction& getExaggerationAction() { return _exaggerationAction; };
    IntegralAction& getExponentialDecayAction() { return _exponentialDecayAction; };
    IntegralAction& getNumTreesAction() { return _numTreesAction; };
    IntegralAction& getNumChecksAction() { return _numChecksAction; };

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
    TsneSettingsAction&     _tsneSettingsAction;        /** Pointer to parent tSNE settings action */
    IntegralAction          _exaggerationAction;        /** Exaggeration action */
    IntegralAction          _exponentialDecayAction;    /** Exponential decay action */
    IntegralAction          _numTreesAction;            /** Exponential decay action */
    IntegralAction          _numChecksAction;           /** Exponential decay action */

    friend class Widget;
};
