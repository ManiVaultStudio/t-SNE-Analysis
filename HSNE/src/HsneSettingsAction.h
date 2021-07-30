#pragma once

#include "actions/Actions.h"

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

protected:
    HsneAnalysisPlugin*         _hsneAnalysisPlugin;                                /** Pointer to HSNE analysis plugin */
    hdps::gui::OptionAction     _knnTypeAction;                                     /** KNN action */
    hdps::gui::IntegralAction   _seedAction;                                        /** Random seed action */
    hdps::gui::ToggleAction     _useMonteCarloSamplingAction;                       /** Use Monte Carlo sampling on/off action */
    hdps::gui::IntegralAction   _numWalksForLandmarkSelectionAction;                /** Number of walks for landmark selection action */
    hdps::gui::DecimalAction    _numWalksForLandmarkSelectionThresholdAction;       /** Number of walks for landmark selection threshold action */
    hdps::gui::IntegralAction   _randomWalkLengthAction;                            /** Random walk length action */
    hdps::gui::IntegralAction   _numWalksForAreaOfInfluenceAction;                  /** Number of walks for area of influence action */
    hdps::gui::IntegralAction   _minWalksRequiredAction;                            /** Minimum number of walks required action */
    hdps::gui::IntegralAction   _numChecksAknnAction;                               /** Number of KNN checks action */
    hdps::gui::ToggleAction     _useOutOfCoreComputationAction;                     /** Use out of core computation action */
	
    friend class Widget;
};