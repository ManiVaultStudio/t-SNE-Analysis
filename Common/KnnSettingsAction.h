#pragma once

#include "actions/GroupAction.h"
#include "actions/IntegralAction.h"

using namespace mv::gui;

class TsneParameters;

/**
 * kNN Settings setting action class
 *
 * Action class for kNN Settings settings
 *
 * @author Thomas Kroes
 */
class KnnSettingsAction : public GroupAction
{
public:

    /**
     * Constructor
     * @param tsneSettingsAction Reference to TSNE settings action
     */
    KnnSettingsAction(QObject* parent, TsneParameters& tsneParameters);

public: // Action getters

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
    TsneParameters&         _tsneParameters;            /** Pointer to tSNE parameters */
    IntegralAction          _numTreesAction;            /** Exponential decay action */
    IntegralAction          _numChecksAction;           /** Exponential decay action */

    friend class Widget;
};
