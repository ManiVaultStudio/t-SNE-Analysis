#pragma once

#include "actions/Actions.h"

class TsneAnalysisPlugin;
class QMenu;

class TsneSettingsAction : public hdps::gui::WidgetAction
{
protected:

    class Widget : public hdps::gui::WidgetActionGroup::Widget {
    public:
        Widget(QWidget* parent, TsneSettingsAction* tsneSettingsAction, const Widget::State& state);
    };

    QWidget* getWidget(QWidget* parent, const Widget::State& state = Widget::State::Standard) override {
        return new Widget(parent, this, state);
    };

public:
    TsneSettingsAction(TsneAnalysisPlugin* tsneAnalysisPlugin);

    QMenu* getContextMenu();

protected:
    TsneAnalysisPlugin*         _tsneAnalysisPlugin;        /** Pointer to TSNE analysis plugin */

    friend class Widget;
};