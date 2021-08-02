#pragma once

#include "actions/Actions.h"

class QMenu;
class HsneAnalysisPlugin;

class GeneralHsneSettingsAction : public hdps::gui::WidgetActionGroup
{
protected:

    class Widget : public hdps::gui::WidgetActionGroup::Widget {
    public:
        Widget(QWidget* parent, GeneralHsneSettingsAction* generalHsneSettingsAction, const Widget::State& state);
    };

    QWidget* getWidget(QWidget* parent, const Widget::State& state = Widget::State::Standard) override {
        return new Widget(parent, this, state);
    };

public:
    GeneralHsneSettingsAction(HsneAnalysisPlugin* hsneAnalysisPlugin);

    QMenu* getContextMenu();

protected:
    HsneAnalysisPlugin*         _hsneAnalysisPlugin;                                /** Pointer to HSNE analysis plugin */
    hdps::gui::OptionAction     _knnTypeAction;                                     /** KNN action */
    hdps::gui::IntegralAction   _seedAction;                                        /** Random seed action */
    hdps::gui::ToggleAction     _useMonteCarloSamplingAction;                       /** Use Monte Carlo sampling on/off action */
    
    friend class Widget;
};