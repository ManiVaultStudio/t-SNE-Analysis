#pragma once

#include "actions/GroupAction.h"
#include "actions/IntegralAction.h"

using namespace mv::gui;

class KnnParameters;

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
    KnnSettingsAction(QObject* parent, KnnParameters& knnParameters);

public: // Action getters

    IntegralAction& getNumTreesAction() { return _numTreesAction; };
    IntegralAction& getNumChecksAction() { return _numChecksAction; };
    IntegralAction& getMAction() { return _mAction; };
    IntegralAction& getEfAction() { return _efAction; };

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
    KnnParameters&          _knnParameters;             /** Pointer to Knn parameters */
    IntegralAction          _numTreesAction;            /** Annoy parameter Trees  action */
    IntegralAction          _numChecksAction;           /** Annoy parameter Checks action */
    IntegralAction          _mAction;                   /** HNSW parameter M action */
    IntegralAction          _efAction;                  /** HNSW parameter ef action */

    friend class Widget;
};
