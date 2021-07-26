#include "DimensionsSettingsAction.h"
#include "TsneAnalysisPlugin.h"
#include "Application.h"

#include <QLabel>

using namespace hdps::gui;

DimensionsSettingsAction::DimensionsSettingsAction(TsneAnalysisPlugin* tsneAnalysisPlugin) :
	WidgetActionGroup(tsneAnalysisPlugin),
	_tsneAnalysisPlugin(tsneAnalysisPlugin)
{
	setText("Dimensions");
}

QMenu* DimensionsSettingsAction::getContextMenu()
{
    auto menu = new QMenu(text());
    return menu;
}

DimensionsSettingsAction::Widget::Widget(QWidget* parent, DimensionsSettingsAction* dimensionsSettingsAction, const Widget::State& state) :
    WidgetAction::Widget(parent, dimensionsSettingsAction, state)
{
    auto layout = new QGridLayout();

    switch (state)
    {
        case Widget::State::Standard:
            layout->setMargin(0);
            setLayout(layout);
            break;

        case Widget::State::Popup:
            setPopupLayout(layout);
            break;

        default:
            break;
    }
}