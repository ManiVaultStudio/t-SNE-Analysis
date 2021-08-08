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
    _topLevelScaleAction(this, _tsneSettingsAction, hsneAnalysisPlugin->getHierarchy(), hsneAnalysisPlugin->getInputDatasetName(), hsneAnalysisPlugin->getOutputDatasetName()),
    _tsneSettingsAction(this),
    _dimensionSelectionAction(this)
{
    setText("HSNE");

    _startAction.setToolTip("Initialize the HSNE hierarchy and create an embedding");

    const auto updateReadOnly = [this]() -> void {
        _startAction.setEnabled(!isReadOnly());

        _generalHsneSettingsAction.setReadOnly(isReadOnly());
        _advancedHsneSettingsAction.setReadOnly(isReadOnly());
        _topLevelScaleAction.setReadOnly(isReadOnly());
        _tsneSettingsAction.setReadOnly(isReadOnly());
        _dimensionSelectionAction.setReadOnly(isReadOnly());
    };

    connect(this, &WidgetActionGroup::readOnlyChanged, this, [this, updateReadOnly](const bool& readOnly) {
        updateReadOnly();
    });

    updateReadOnly();
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
