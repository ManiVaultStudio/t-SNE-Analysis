#pragma once

#include "TsneParameters.h"
#include "GeneralTsneSettingsAction.h"
#include "AdvancedTsneSettingsAction.h"

class QMenu;

class TsneSettingsAction : public hdps::gui::WidgetAction
{
protected:

    class Widget : public hdps::gui::WidgetAction::Widget {
    public:
        Widget(QWidget* parent, TsneSettingsAction* tsneSettingsAction, const Widget::State& state);
    };

    QWidget* getWidget(QWidget* parent, const Widget::State& state = Widget::State::Standard) override {
        return new Widget(parent, this, state);
    };

public:
    TsneSettingsAction(QObject* parent);

    QMenu* getContextMenu();

    TsneParameters& getTsneParameters() { return _tsneParameters; }
    hdps::gui::TriggerAction& getStartComputationAction() { return _startComputationAction; }
    hdps::gui::TriggerAction& getContinueComputationAction() { return _continueComputationAction; }
    hdps::gui::TriggerAction& getStopComputationAction() { return _stopComputationAction; }
    hdps::gui::ToggleAction& getRunningAction() { return _runningAction; }
    GeneralTsneSettingsAction& getGeneralTsneSettingsAction() { return _generalTsneSettingsAction; }
    AdvancedTsneSettingsAction& getAdvancedTsneSettingsAction() { return _advancedTsneSettingsAction; }

protected:
    TsneParameters                  _tsneParameters;                /** TSNE parameters */
    hdps::gui::TriggerAction        _startComputationAction;        /** Start computation action */
    hdps::gui::TriggerAction        _continueComputationAction;     /** Start computation action */
    hdps::gui::TriggerAction        _stopComputationAction;         /** Stop computation action */
    hdps::gui::ToggleAction         _runningAction;                 /** Running action */
    GeneralTsneSettingsAction       _generalTsneSettingsAction;     /** General tSNE settings action */
    AdvancedTsneSettingsAction      _advancedTsneSettingsAction;    /** Advanced tSNE settings action */

    friend class Widget;
};