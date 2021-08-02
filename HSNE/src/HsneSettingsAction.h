#pragma once

#include "HsneParameters.h"
#include "TsneParameters.h"
#include "GeneralHsneSettingsAction.h"
#include "AdvancedHsneSettingsAction.h"
#include "TsneSettingsAction.h"
#include "DimensionSelectionAction.h"

class QMenu;
class HsneAnalysisPlugin;

class HsneSettingsAction : public hdps::gui::WidgetActionGroup
{
protected:

    class Widget : public hdps::gui::WidgetActionGroup::Widget {
    public:
        Widget(QWidget* parent, HsneSettingsAction* hsneSettingsAction, const Widget::State& state);
    };

    QWidget* getWidget(QWidget* parent, const Widget::State& state = Widget::State::Standard) override {
        return new Widget(parent, this, state);
    };

public:
    HsneSettingsAction(HsneAnalysisPlugin* hsneAnalysisPlugin);

    QMenu* getContextMenu();

    HsneParameters& getHsneParameters();
    TsneParameters& getTsneParameters();

    GeneralHsneSettingsAction& getGeneralHsneSettingsAction() { return _generalHsneSettingsAction; }
    AdvancedHsneSettingsAction& getAdvancedHsneSettingsAction() { return _advancedHsneSettingsAction; }
    TsneSettingsAction& getTsneSettingsAction() { return _tsneSettingsAction; }
    DimensionSelectionAction& getDimensionSelectionAction() { return _dimensionSelectionAction; }
    hdps::gui::TriggerAction& getStartComputationAction() { return _startComputationAction; }
    hdps::gui::TriggerAction& getContinueComputationAction() { return _continueComputationAction; }
    hdps::gui::TriggerAction& getStopComputationAction() { return _stopComputationAction; }

protected:
    HsneAnalysisPlugin*             _hsneAnalysisPlugin;            /** Pointer to HSNE analysis plugin */
    HsneParameters                  _hsneParameters;                /** HSNE parameters */
    TsneParameters                  _tsneParameters;                /** TSNE parameters */
    GeneralHsneSettingsAction       _generalHsneSettingsAction;     /** General HSNE settings action */
    AdvancedHsneSettingsAction      _advancedHsneSettingsAction;    /** Advanced HSNE settings action */
    TsneSettingsAction              _tsneSettingsAction;            /** TSNE settings action */
    DimensionSelectionAction        _dimensionSelectionAction;      /** Dimension selection action */
    hdps::gui::TriggerAction        _startComputationAction;        /** Start computation action */
    hdps::gui::TriggerAction        _continueComputationAction;     /** Continue computation action */
    hdps::gui::TriggerAction        _stopComputationAction;         /** Stop computation action */

    friend class Widget;
};