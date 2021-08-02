#include "HsneSettingsAction.h"
#include "HsneAnalysisPlugin.h"
#include "Application.h"

using namespace hdps::gui;

HsneSettingsAction::HsneSettingsAction(HsneAnalysisPlugin* hsneAnalysisPlugin) :
    WidgetActionGroup(hsneAnalysisPlugin, true),
    _hsneAnalysisPlugin(hsneAnalysisPlugin),
    _hsneParameters(),
    _tsneParameters(),
    _generalHsneSettingsAction(hsneAnalysisPlugin),
    _advancedHsneSettingsAction(hsneAnalysisPlugin),
    _tsneSettingsAction(this),
    _dimensionSelectionAction(this)
{
    setText("HSNE");
}

QMenu* HsneSettingsAction::getContextMenu()
{
    auto menu = new QMenu(text());

    /*
    menu->addAction(&_startComputationAction);
    menu->addAction(&_stopComputationAction);

    menu->addSeparator();

    const auto addSetting = [this, menu](WidgetAction& widgetAction) -> void {
        auto settingMenu = new QMenu(widgetAction.text());
        settingMenu->addAction(&widgetAction);
        menu->addMenu(settingMenu);
    };

    addSetting(_knnTypeAction);
    addSetting(_distanceMetricAction);
    addSetting(_numIterationsAction);

    menu->addSeparator();

    menu->addAction(&_resetAction);
    */

    return menu;
}

HsneParameters& HsneSettingsAction::getHsneParameters()
{
    return _hsneParameters;
}

TsneParameters& HsneSettingsAction::getTsneParameters()
{
    return _tsneParameters;
}

HsneSettingsAction::Widget::Widget(QWidget* parent, HsneSettingsAction* hsneSettingsAction, const Widget::State& state) :
    WidgetAction::Widget(parent, hsneSettingsAction, state)
{
    auto layout = new QGridLayout();

    /*
    const auto addOptionActionToLayout = [this, layout](OptionAction& optionAction) -> void {
        const auto numRows = layout->rowCount();

        layout->addWidget(optionAction.createLabelWidget(this), numRows, 0);
        layout->addWidget(optionAction.createWidget(this), numRows, 1);
    };

    const auto addIntegralActionToLayout = [this, layout](IntegralAction& integralAction) -> void {
        const auto numRows = layout->rowCount();

        layout->addWidget(integralAction.createLabelWidget(this), numRows, 0);
        layout->addWidget(integralAction.createWidget(this), numRows, 1);
    };

    addOptionActionToLayout(hsneSettingsAction->_knnTypeAction);
    addOptionActionToLayout(hsneSettingsAction->_distanceMetricAction);
    addIntegralActionToLayout(hsneSettingsAction->_numIterationsAction);
    addIntegralActionToLayout(hsneSettingsAction->_perplexityAction);

    layout->addWidget(hsneSettingsAction->_resetAction.createWidget(this), layout->rowCount(), 1, 1, 2);

    auto computeLayout = new QHBoxLayout();

    computeLayout->addWidget(hsneSettingsAction->_startComputationAction.createWidget(this));
    computeLayout->addWidget(hsneSettingsAction->_continueComputationAction.createWidget(this));
    computeLayout->addWidget(hsneSettingsAction->_stopComputationAction.createWidget(this));

    layout->addLayout(computeLayout, layout->rowCount(), 1, 1, 2);

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
    */
}