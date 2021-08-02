#pragma once

#include "actions/Actions.h"

class QMenu;
class HsneSettingsAction;

class AdvancedHsneSettingsAction : public hdps::gui::WidgetActionGroup
{
protected:

    class Widget : public hdps::gui::WidgetActionGroup::Widget {
    public:
        Widget(QWidget* parent, AdvancedHsneSettingsAction* advancedHsneSettingsAction, const Widget::State& state);
    };

    QWidget* getWidget(QWidget* parent, const Widget::State& state = Widget::State::Standard) override {
        return new Widget(parent, this, state);
    };

public:
    AdvancedHsneSettingsAction(HsneSettingsAction& hsneSettingsAction);

    QMenu* getContextMenu();

    HsneSettingsAction& getHsneSettingsAction() { return _hsneSettingsAction; }

protected:
    HsneSettingsAction&         _hsneSettingsAction;                                /** Reference to HSNE settings action */
    hdps::gui::IntegralAction   _numWalksForLandmarkSelectionAction;                /** Number of walks for landmark selection action */
    hdps::gui::DecimalAction    _numWalksForLandmarkSelectionThresholdAction;       /** Number of walks for landmark selection threshold action */
    hdps::gui::IntegralAction   _randomWalkLengthAction;                            /** Random walk length action */
    hdps::gui::IntegralAction   _numWalksForAreaOfInfluenceAction;                  /** Number of walks for area of influence action */
    hdps::gui::IntegralAction   _minWalksRequiredAction;                            /** Minimum number of walks required action */
    hdps::gui::IntegralAction   _numChecksAknnAction;                               /** Number of KNN checks action */
    hdps::gui::ToggleAction     _useOutOfCoreComputationAction;                     /** Use out of core computation action */

    friend class Widget;
};