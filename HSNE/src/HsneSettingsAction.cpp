#include "HsneSettingsAction.h"
#include "HsneAnalysisPlugin.h"

using namespace hdps::gui;

HsneSettingsAction::HsneSettingsAction(HsneAnalysisPlugin* hsneAnalysisPlugin) :
    WidgetActionGroup(hsneAnalysisPlugin, true),
    _hsneAnalysisPlugin(hsneAnalysisPlugin),
    _hsneParameters(),
    _tsneParameters(),
    _startStopAction(this, "Start"),
    _generalHsneSettingsAction(*this),
    _advancedHsneSettingsAction(*this),
    _tsneSettingsAction(this),
    _dimensionSelectionAction(this)
{
    setText("HSNE");

    connect(&_startStopAction, &ToggleAction::toggled, this, [this]() {
        const auto prefix = "Start";// _startStopAction.isChecked() ? "Stop" : "Start";

        _startStopAction.setText(prefix);
        _startStopAction.setToolTip(QString("%1 the HSNE computation").arg(prefix));
    });
}

QMenu* HsneSettingsAction::getContextMenu()
{
    return nullptr;
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