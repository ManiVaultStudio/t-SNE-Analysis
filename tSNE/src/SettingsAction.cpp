#include "SettingsAction.h"
#include "Application.h"
#include "TsneAnalysisPlugin.h"

using namespace hdps::gui;

SettingsAction::SettingsAction(TsneAnalysisPlugin* tsneAnalysisPlugin) :
    WidgetAction(tsneAnalysisPlugin),
	_startComputationAction(this, "Start computation"),
	_stopComputationAction(this, "Stop computation")
{
	setText("Settings");
}

QMenu* SettingsAction::getContextMenu()
{
    auto menu = new QMenu("TSNE Analysis");

    menu->addAction(&_startComputationAction);
    menu->addAction(&_stopComputationAction);

    return menu;
}

SettingsAction::Widget::Widget(QWidget* parent, SettingsAction* settingsAction, const Widget::State& state) :
    WidgetAction::Widget(parent, settingsAction, state)
{
    auto layout = new QVBoxLayout();

    layout->addWidget(settingsAction->_startComputationAction.createWidget(this));
    layout->addWidget(settingsAction->_stopComputationAction.createWidget(this));

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