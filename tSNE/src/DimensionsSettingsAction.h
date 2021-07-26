#pragma once

#include "actions/Actions.h"

class QMenu;
class TsneAnalysisPlugin;

class DimensionsSettingsAction : public hdps::gui::WidgetActionGroup
{
protected:

    class Widget : public hdps::gui::WidgetActionGroup::Widget {
    public:
        Widget(QWidget* parent, DimensionsSettingsAction* dimensionsSettingsAction, const Widget::State& state);
    };

    QWidget* getWidget(QWidget* parent, const Widget::State& state = Widget::State::Standard) override {
        return new Widget(parent, this, state);
    };

public:
	DimensionsSettingsAction(TsneAnalysisPlugin* tsneAnalysisPlugin);

    QMenu* getContextMenu();

protected:
	TsneAnalysisPlugin*			_tsneAnalysisPlugin;		/** Pointer to TSNE analysis plugin */
	
    friend class Widget;
};