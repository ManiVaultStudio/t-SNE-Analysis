#pragma once

#include "actions/Actions.h"

class QMenu;
class HsneSettingsAction;

/**
 * Advanced HSNE setting action class
 *
 * Action class for advanced HSNE settings
 *
 * @author Thomas Kroes
 */
class AdvancedHsneSettingsAction : public hdps::gui::WidgetActionGroup
{
protected:

    /** Widget class for advanced HSNE settings action */
    class Widget : public hdps::gui::WidgetActionGroup::FormWidget {
    public:

        /**
         * Constructor
         * @param parent Pointer to parent widget
         * @param advancedHsneSettingsAction Pointer to advanced HSNE settings action
         * @param state State of the widget
         */
        Widget(QWidget* parent, AdvancedHsneSettingsAction* advancedHsneSettingsAction, const Widget::State& state);
    };

    /**
     * Get widget representation of the advanced HSNE action
     * @param parent Pointer to parent widget
     * @param state Widget state
     */
    QWidget* getWidget(QWidget* parent, const hdps::gui::WidgetActionWidget::State& state = hdps::gui::WidgetActionWidget::State::Standard) override {
        return new Widget(parent, this, state);
    };

public:

    /**
     * Constructor
     * @param hsneSettingsAction Reference to HSNE settings action
     */
    AdvancedHsneSettingsAction(HsneSettingsAction& hsneSettingsAction);

public: // Action getters

    HsneSettingsAction& getHsneSettingsAction() { return _hsneSettingsAction; }
    hdps::gui::IntegralAction& getNumWalksForLandmarkSelectionAction() { return _numWalksForLandmarkSelectionAction; }
    hdps::gui::DecimalAction& getNumWalksForLandmarkSelectionThresholdAction() { return _numWalksForLandmarkSelectionThresholdAction; }
    hdps::gui::IntegralAction& getRandomWalkLengthAction() { return _randomWalkLengthAction; }
    hdps::gui::IntegralAction& getNumWalksForAreaOfInfluenceAction() { return _numWalksForAreaOfInfluenceAction; }
    hdps::gui::IntegralAction& getMinWalksRequiredAction() { return _minWalksRequiredAction; }
    hdps::gui::IntegralAction& getNumChecksAknnAction() { return _numChecksAknnAction; }
    hdps::gui::ToggleAction& getUseOutOfCoreComputationAction() { return _useOutOfCoreComputationAction; }

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
