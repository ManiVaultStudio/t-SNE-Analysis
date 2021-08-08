#include "HsneSettingsAction.h"
#include "HsneAnalysisPlugin.h"

using namespace hdps::gui;

HsneSettingsAction::HsneSettingsAction(HsneAnalysisPlugin* hsneAnalysisPlugin) :
    WidgetActionGroup(hsneAnalysisPlugin, true),
    _hsneAnalysisPlugin(hsneAnalysisPlugin),
    _hsneParameters(),
    _tsneParameters(),
    _startAction(this, "Start"),
    _generalHsneSettingsAction(*this),
    _advancedHsneSettingsAction(*this),
    _tsneSettingsAction(this),
    _dimensionSelectionAction(this)
{
    setText("HSNE");

    connect(&_startAction, &ToggleAction::toggled, this, [this]() {
        const auto prefix = "Start";// _startStopAction.isChecked() ? "Stop" : "Start";

        _startAction.setText(prefix);
        _startAction.setToolTip(QString("%1 the HSNE computation").arg(prefix));
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
    WidgetActionGroup::GroupWidget(parent, hsneSettingsAction)
{
}