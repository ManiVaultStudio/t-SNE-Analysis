#pragma once

#include "AdvancedHsneSettingsAction.h"
#include "GeneralHsneSettingsAction.h"
#include "HsneParameters.h"
#include "HsneScaleAction.h"
#include "TsneParameters.h"
#include "TsneSettingsAction.h"

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
    HsneParameters& getHsneParameters();
    TsneParameters& getTsneParameters();

public: // Action getters

    GeneralHsneSettingsAction& getGeneralHsneSettingsAction() { return _generalHsneSettingsAction; }
    AdvancedHsneSettingsAction& getAdvancedHsneSettingsAction() { return _advancedHsneSettingsAction; }
    HsneScaleAction& getTopLevelScaleAction() { return _topLevelScaleAction; }
    TsneSettingsAction& getTsneSettingsAction() { return _tsneSettingsAction; }

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
    HsneAnalysisPlugin*             _hsneAnalysisPlugin;            /** Pointer to HSNE analysis plugin */
    HsneParameters                  _hsneParameters;                /** HSNE parameters */
    GeneralHsneSettingsAction       _generalHsneSettingsAction;     /** General HSNE settings action */
    AdvancedHsneSettingsAction      _advancedHsneSettingsAction;    /** Advanced HSNE settings action */
    HsneScaleAction                 _topLevelScaleAction;           /** Top level scale action */
    TsneSettingsAction              _tsneSettingsAction;            /** TSNE settings action */

    friend class Widget;
};
