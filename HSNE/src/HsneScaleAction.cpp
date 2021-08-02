#include "HsneScaleAction.h"
#include "HsneAnalysisPlugin.h"
#include "HsneHierarchy.h"

using namespace hdps::gui;

HsneScaleAction::HsneScaleAction(HsneAnalysisPlugin* hsneAnalysisPlugin, HsneHierarchy* hsneHierarchy) :
    WidgetActionGroup(hsneAnalysisPlugin, true),
    _hsneAnalysisPlugin(hsneAnalysisPlugin),
    _hsneHierarchy(hsneHierarchy)
{
    setText("HSNE scale");
}

QMenu* HsneScaleAction::getContextMenu()
{
    return nullptr;
}

HsneScaleAction::Widget::Widget(QWidget* parent, HsneScaleAction* HsneScaleAction, const Widget::State& state) :
    WidgetAction::Widget(parent, HsneScaleAction, state)
{
}
