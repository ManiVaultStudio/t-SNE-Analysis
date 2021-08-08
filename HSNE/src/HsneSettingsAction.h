#pragma once

#include "HsneParameters.h"
#include "TsneParameters.h"
#include "GeneralHsneSettingsAction.h"
#include "AdvancedHsneSettingsAction.h"
#include "HsneScaleAction.h"
#include "TsneSettingsAction.h"
#include "DimensionSelectionAction.h"

class QMenu;
class HsneAnalysisPlugin;

class HsneSettingsAction : public hdps::gui::WidgetActionGroup
{
public:
    HsneSettingsAction(HsneAnalysisPlugin* hsneAnalysisPlugin);

    QMenu* getContextMenu();

    /** Get HSNE/TSNE parameters */
    HsneParameters& getHsneParameters();
    TsneParameters& getTsneParameters();

    hdps::gui::TriggerAction& getStartAction() { return _startAction; }

    GeneralHsneSettingsAction& getGeneralHsneSettingsAction() { return _generalHsneSettingsAction; }
    AdvancedHsneSettingsAction& getAdvancedHsneSettingsAction() { return _advancedHsneSettingsAction; }
    HsneScaleAction& getTopLevelScaleAction() { return _topLevelScaleAction; }
    TsneSettingsAction& getTsneSettingsAction() { return _tsneSettingsAction; }
    DimensionSelectionAction& getDimensionSelectionAction() { return _dimensionSelectionAction; }

protected:
    HsneAnalysisPlugin*             _hsneAnalysisPlugin;            /** Pointer to HSNE analysis plugin */
    HsneParameters                  _hsneParameters;                /** HSNE parameters */
    TsneParameters                  _tsneParameters;                /** TSNE parameters */
    hdps::gui::TriggerAction        _startAction;                   /** Start action */
    GeneralHsneSettingsAction       _generalHsneSettingsAction;     /** General HSNE settings action */
    AdvancedHsneSettingsAction      _advancedHsneSettingsAction;    /** Advanced HSNE settings action */
    HsneScaleAction                 _topLevelScaleAction;           /** Top level scale action */
    TsneSettingsAction              _tsneSettingsAction;            /** TSNE settings action */
    DimensionSelectionAction        _dimensionSelectionAction;      /** Dimension selection action */

    friend class Widget;
};