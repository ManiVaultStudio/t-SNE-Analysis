#pragma once

#include "HsneParameters.h"
#include "TsneParameters.h"
#include "GeneralHsneSettingsAction.h"
#include "AdvancedHsneSettingsAction.h"
#include "HsneScaleAction.h"
#include "TsneSettingsAction.h"

using namespace mv::gui;

class QMenu;
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

protected:
    HsneAnalysisPlugin*             _hsneAnalysisPlugin;            /** Pointer to HSNE analysis plugin */
    HsneParameters                  _hsneParameters;                /** HSNE parameters */
    TsneParameters                  _tsneParameters;                /** TSNE parameters */
    GeneralHsneSettingsAction       _generalHsneSettingsAction;     /** General HSNE settings action */
    AdvancedHsneSettingsAction      _advancedHsneSettingsAction;    /** Advanced HSNE settings action */
    HsneScaleAction                 _topLevelScaleAction;           /** Top level scale action */
    TsneSettingsAction              _tsneSettingsAction;            /** TSNE settings action */

    friend class Widget;
};
