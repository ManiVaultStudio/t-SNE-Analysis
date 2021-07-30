#include "TsneSettingsAction.h"
#include "TsneAnalysisPlugin.h"

using namespace hdps::gui;

TsneSettingsAction::TsneSettingsAction(TsneAnalysisPlugin* tsneAnalysisPlugin) :
    WidgetAction(tsneAnalysisPlugin),
    _tsneAnalysisPlugin(tsneAnalysisPlugin)
{
    setText("TSNE");
}

QMenu* TsneSettingsAction::getContextMenu()
{
    auto menu = new QMenu(text());

    menu->addAction(&_tsneAnalysisPlugin->getGeneralSettingsAction().getStartComputationAction());
    menu->addAction(&_tsneAnalysisPlugin->getGeneralSettingsAction().getRunningAction());
    menu->addAction(&_tsneAnalysisPlugin->getGeneralSettingsAction().getStopComputationAction());

    return menu;
}

TsneSettingsAction::Widget::Widget(QWidget* parent, TsneSettingsAction* tsneSettingsAction, const Widget::State& state) :
    WidgetAction::Widget(parent, tsneSettingsAction, state)
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