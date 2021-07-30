#include "ContextMenuAction.h"
#include "TsneAnalysisPlugin.h"

using namespace hdps::gui;

ContextMenuAction::ContextMenuAction(TsneAnalysisPlugin* tsneAnalysisPlugin) :
    WidgetAction(tsneAnalysisPlugin),
    _tsneAnalysisPlugin(tsneAnalysisPlugin)
{
    setText("TSNE");
}

QMenu* ContextMenuAction::getContextMenu()
{
    auto menu = new QMenu(text());

    menu->addAction(&_tsneAnalysisPlugin->getGeneralSettingsAction().getStartComputationAction());
    menu->addAction(&_tsneAnalysisPlugin->getGeneralSettingsAction().getRunningAction());
    menu->addAction(&_tsneAnalysisPlugin->getGeneralSettingsAction().getStopComputationAction());

    return menu;
}

ContextMenuAction::Widget::Widget(QWidget* parent, ContextMenuAction* contextMenuAction, const Widget::State& state) :
    WidgetAction::Widget(parent, contextMenuAction, state)
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