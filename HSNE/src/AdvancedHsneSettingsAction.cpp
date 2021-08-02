#include "AdvancedHsneSettingsAction.h"
#include "HsneAnalysisPlugin.h"
#include "Application.h"

using namespace hdps::gui;

AdvancedHsneSettingsAction::AdvancedHsneSettingsAction(HsneAnalysisPlugin* hsneAnalysisPlugin) :
    WidgetActionGroup(hsneAnalysisPlugin),
    _hsneAnalysisPlugin(hsneAnalysisPlugin),
    _numWalksForLandmarkSelectionAction(this, "No. walks for landmark selection"),
    _numWalksForLandmarkSelectionThresholdAction(this, "No. walks for landmark selection threshold"),
    _randomWalkLengthAction(this, "Random walk length"),
    _numWalksForAreaOfInfluenceAction(this, "No. walks for area of influence"),
    _minWalksRequiredAction(this, "Minimum no. walks required"),
    _numChecksAknnAction(this, "No. KNN checks", 0, 1024, 512, 512),
    _useOutOfCoreComputationAction(this, "Use out-of-core computation")
{
    setText("HSNE (advanced)");
}

QMenu* AdvancedHsneSettingsAction::getContextMenu()
{
    return nullptr;
}

AdvancedHsneSettingsAction::Widget::Widget(QWidget* parent, AdvancedHsneSettingsAction* advancedHsneSettingsAction, const Widget::State& state) :
    WidgetAction::Widget(parent, advancedHsneSettingsAction, state)
{
    auto layout = new QGridLayout();

    const auto addActionToLayout = [this, layout](WidgetAction* widgetAction) -> void {
        const auto numRows = layout->rowCount();

        if (dynamic_cast<ToggleAction*>(widgetAction) == nullptr)
            layout->addWidget(widgetAction->createLabelWidget(this), numRows, 0);

        layout->addWidget(widgetAction->createWidget(this), numRows, 1);
    };

    addActionToLayout(&advancedHsneSettingsAction->_numWalksForLandmarkSelectionAction);
    addActionToLayout(&advancedHsneSettingsAction->_numWalksForLandmarkSelectionThresholdAction);
    addActionToLayout(&advancedHsneSettingsAction->_randomWalkLengthAction);
    addActionToLayout(&advancedHsneSettingsAction->_numWalksForAreaOfInfluenceAction);
    addActionToLayout(&advancedHsneSettingsAction->_minWalksRequiredAction);
    addActionToLayout(&advancedHsneSettingsAction->_numChecksAknnAction);
    addActionToLayout(&advancedHsneSettingsAction->_useOutOfCoreComputationAction);

    switch (state)
    {
        case Widget::State::Standard:
            setLayout(layout);
            break;

        case Widget::State::Popup:
            setPopupLayout(layout);
            break;

        default:
            break;
    }
}