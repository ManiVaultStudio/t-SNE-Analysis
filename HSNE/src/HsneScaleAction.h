#pragma once

#include "actions/Actions.h"

class QMenu;
class HsneAnalysisPlugin;
class HsneHierarchy;

class HsneScaleAction : public hdps::gui::WidgetActionGroup
{
protected:

    class Widget : public hdps::gui::WidgetActionGroup::Widget {
    public:
        Widget(QWidget* parent, HsneScaleAction* HsneScaleAction, const Widget::State& state);
    };

    QWidget* getWidget(QWidget* parent, const Widget::State& state = Widget::State::Standard) override {
        return new Widget(parent, this, state);
    };

public:
    HsneScaleAction(HsneAnalysisPlugin* hsneAnalysisPlugin, HsneHierarchy* hsneHierarchy);

    QMenu* getContextMenu();

protected:
    HsneAnalysisPlugin*     _hsneAnalysisPlugin;        /** Pointer to HSNE analysis plugin */
    HsneHierarchy*          _hsneHierarchy;             /** Pointer to HSNE hierarchy */

    friend class Widget;
};