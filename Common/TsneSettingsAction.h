#pragma once

#include "TsneParameters.h"
#include "TsneComputationAction.h"
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
    TsneComputationAction& getComputationAction() { return _computationAction; }
    GeneralTsneSettingsAction& getGeneralTsneSettingsAction() { return _generalTsneSettingsAction; }
    AdvancedTsneSettingsAction& getAdvancedTsneSettingsAction() { return _advancedTsneSettingsAction; }

protected:
    TsneParameters                  _tsneParameters;                /** TSNE parameters */
    TsneComputationAction           _computationAction;             /** Computation action */
    GeneralTsneSettingsAction       _generalTsneSettingsAction;     /** General tSNE settings action */
    AdvancedTsneSettingsAction      _advancedTsneSettingsAction;    /** Advanced tSNE settings action */

    friend class Widget;
};