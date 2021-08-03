#pragma once

#include "actions/Actions.h"

class QMenu;
class TsneSettingsAction;

class AdvancedTsneSettingsAction : public hdps::gui::WidgetActionGroup
{
protected:

    class Widget : public hdps::gui::WidgetActionGroup::Widget {
    public:
        Widget(QWidget* parent, AdvancedTsneSettingsAction* advancedSettingsAction, const Widget::State& state);
    };

    QWidget* getWidget(QWidget* parent, const Widget::State& state = Widget::State::Standard) override {
        return new Widget(parent, this, state);
    };

public:
    AdvancedTsneSettingsAction(TsneSettingsAction& tsneSettingsAction);

    QMenu* getContextMenu() { return nullptr; }

    /**
     * Sets the group read-only
     * @param readOnly Whether the group is read-only
     */
    void setReadOnly(const bool& readOnly) override;

protected:
    TsneSettingsAction&         _tsneSettingsAction;        /** Pointer to parent tSNE settings action */
    hdps::gui::IntegralAction   _exaggerationAction;        /** Exaggeration action */
    hdps::gui::IntegralAction   _exponentialDecayAction;    /** Exponential decay action */
    hdps::gui::IntegralAction   _numTreesAction;            /** Exponential decay action */
    hdps::gui::IntegralAction   _numChecksAction;           /** Exponential decay action */
    hdps::gui::TriggerAction    _resetAction;               /** Reset all input to defaults */

    friend class Widget;
};