#include "TsneSettingsAction.h"

#include <QTabWidget>

using namespace hdps::gui;

TsneSettingsAction::TsneSettingsAction(QObject* parent) :
    WidgetAction(parent),
    _tsneParameters(),
    _generalTsneSettingsAction(*this),
    _advancedTsneSettingsAction(*this),
    _startComputationAction(this, "Start"),
    _continueComputationAction(this, "Continue"),
    _stopComputationAction(this, "Stop"),
    _runningAction(this, "Running")
{
    setText("TSNE");
}

QMenu* TsneSettingsAction::getContextMenu()
{
    auto menu = new QMenu(text());

    menu->addAction(&_startComputationAction);
    menu->addAction(&_continueComputationAction);
    menu->addAction(&_runningAction);

    return menu;
}

TsneSettingsAction::Widget::Widget(QWidget* parent, TsneSettingsAction* tsneSettingsAction, const Widget::State& state) :
    WidgetAction::Widget(parent, tsneSettingsAction, state)
{
    auto layout = new QVBoxLayout();

    /*
    auto tabs = new QTabWidget();

    const auto getTabWidget = [](QWidget* widget) -> QWidget* {
        auto containerWidget    = new QWidget();
        auto containerLayout    = new QVBoxLayout();

        containerLayout->addWidget(widget);
        containerWidget->setLayout(containerLayout);

        return containerWidget;
    };

    tabs->addTab(getTabWidget(tsneSettingsAction->getGeneralTsneSettingsAction().createWidget(this)), "General");
    tabs->addTab(getTabWidget(tsneSettingsAction->getAdvancedTsneSettingsAction().createWidget(this)), "Advanced");

    layout->setMargin(2);
    layout->addWidget(tabs);
    */

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