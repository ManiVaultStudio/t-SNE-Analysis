#pragma once

#include "GeneralHsneSettingsAction.h"
#include "GradientDescentSettingsAction.h"
#include "HierarchyConstructionSettingsAction.h"
#include "HsneParameters.h"
#include "HsneScaleAction.h"
#include "KnnParameters.h"
#include "KnnSettingsAction.h"
#include "TsneParameters.h"

using namespace mv::gui;

class HsneAnalysisPlugin;

/**
 * HSNE setting action class
 *
 * Action class for HSNE settings
 *
 * @author Thomas Kroes
 */
class HsneSettingsAction : public GroupAction
{
public:

    /**
     * Constructor
     * @param hsneAnalysisPlugin Pointer to HSNE analysis plugin
     */
    HsneSettingsAction(HsneAnalysisPlugin* hsneAnalysisPlugin);

    /** Get HSNE/TSNE parameters */
    HsneParameters& getHsneParameters() { return _hsneParameters;  }
    TsneParameters& getTsneParameters() { return _tsneParameters; }
    KnnParameters& getKnnParameters() { return _knnParameters; }

public: // Action getters

    GeneralHsneSettingsAction& getGeneralHsneSettingsAction() { return _generalHsneSettingsAction; }
    HierarchyConstructionSettingsAction& getHierarchyConstructionSettingsAction() { return _hierarchyConstructionSettingsAction; }
    HsneScaleAction& getTopLevelScaleAction() { return _topLevelScaleAction; }
    GradientDescentSettingsAction& getGradientDescentSettingsAction() { return _gradientDescentSettingsAction; }
    KnnSettingsAction& getKnnSettingsAction() { return _knnSettingsAction; }

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

private:
    HsneParameters                          _hsneParameters;                        /** HSNE parameters */
    TsneParameters                          _tsneParameters;                        /** TSNE parameters */
    KnnParameters                           _knnParameters;                         /** Knn parameters */

private:
    HsneAnalysisPlugin*                     _hsneAnalysisPlugin;                    /** Pointer to HSNE analysis plugin */
    GeneralHsneSettingsAction               _generalHsneSettingsAction;             /** General HSNE settings action */
    HierarchyConstructionSettingsAction     _hierarchyConstructionSettingsAction;   /** Hierarchy Construction settings action */
    GradientDescentSettingsAction           _gradientDescentSettingsAction;         /** Gradient descent settings action */
    KnnSettingsAction                       _knnSettingsAction;                     /** knn settings action */
    HsneScaleAction                         _topLevelScaleAction;                   /** Top level scale action */
};
