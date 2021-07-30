#pragma once

#include "actions/Actions.h"

class TsneAnalysisPlugin;
class QMenu;

class ContextMenuAction : public hdps::gui::WidgetAction
{
protected:

    class Widget : public hdps::gui::WidgetActionGroup::Widget {
    public:
        Widget(QWidget* parent, ContextMenuAction* contextMenuAction, const Widget::State& state);
    };

    QWidget* getWidget(QWidget* parent, const Widget::State& state = Widget::State::Standard) override {
        return new Widget(parent, this, state);
    };

public:
    ContextMenuAction(TsneAnalysisPlugin* tsneAnalysisPlugin);

    QMenu* getContextMenu();

protected:
    TsneAnalysisPlugin*         _tsneAnalysisPlugin;        /** Pointer to TSNE analysis plugin */

    friend class Widget;
};