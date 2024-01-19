#pragma once

#include "actions/IntegralAction.h"
#include "actions/OptionAction.h"

#include "TsneComputationAction.h"

using namespace mv::gui;

class TsneSettingsAction;

/**
 * General TSNE setting action class
 *
 * Actions class for general TSNE settings
 *
 * @author Thomas Kroes
 */
class GeneralTsneSettingsAction : public GroupAction
{
public:

    /**
     * Constructor
     * @param tsneSettingsAction Reference to TSNE settings action
     */
    GeneralTsneSettingsAction(TsneSettingsAction& tsneSettingsAction);

public: // Action getters

    TsneSettingsAction& getTsneSettingsAction() { return _tsneSettingsAction; };
    OptionAction& getKnnAlgorithmAction() { return _knnAlgorithmAction; };
    OptionAction& getDistanceMetricAction() { return _distanceMetricAction; };
    IntegralAction& getNumIterationsAction() { return _numIterationsAction; };
    IntegralAction& getNumberOfComputatedIterationsAction() { return _numberOfComputatedIterationsAction; };
    IntegralAction& getPerplexityAction() { return _perplexityAction; };
    TsneComputationAction& getComputationAction() { return _computationAction; }

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
    TsneSettingsAction&     _tsneSettingsAction;                    /** Reference to parent tSNE settings action */
    OptionAction            _knnAlgorithmAction;                    /** KNN algorithm action */
    OptionAction            _distanceMetricAction;                  /** Distance metric action */
    IntegralAction          _numIterationsAction;                   /** Number of iterations action */
    IntegralAction          _numberOfComputatedIterationsAction;    /** Number of computed iterations action */
    IntegralAction          _perplexityAction;                      /** Perplexity action */
    IntegralAction          _updateIterationsAction;                /** Number of update iterations (copying embedding to ManiVault core) */
    TsneComputationAction   _computationAction;                     /** Computation action */
};
