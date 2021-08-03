#include "AdvancedHsneSettingsAction.h"
#include "HsneAnalysisPlugin.h"

using namespace hdps::gui;

AdvancedHsneSettingsAction::AdvancedHsneSettingsAction(HsneSettingsAction& hsneSettingsAction) :
    WidgetActionGroup(&hsneSettingsAction),
    _hsneSettingsAction(hsneSettingsAction),
    _numWalksForLandmarkSelectionAction(this, "No. walks for landmark selection"),
    _numWalksForLandmarkSelectionThresholdAction(this, "No. walks for landmark selection threshold"),
    _randomWalkLengthAction(this, "Random walk length"),
    _numWalksForAreaOfInfluenceAction(this, "No. walks for area of influence"),
    _minWalksRequiredAction(this, "Minimum no. walks required"),
    _numChecksAknnAction(this, "No. KNN checks", 0, 1024, 512, 512),
    _useOutOfCoreComputationAction(this, "Use out-of-core computation")
{
    const auto& hsneParameters = hsneSettingsAction.getHsneParameters();

    _numWalksForLandmarkSelectionAction.setValue(hsneParameters.getNumWalksForLandmarkSelection());
    _numWalksForLandmarkSelectionThresholdAction.setValue(hsneParameters.getNumWalksForLandmarkSelectionThreshold());
    _randomWalkLengthAction.setValue(hsneParameters.getRandomWalkLength());
    _numWalksForAreaOfInfluenceAction.setValue(hsneParameters.getNumWalksForAreaOfInfluence());
    _minWalksRequiredAction.setValue(hsneParameters.getMinWalksRequired());
    _numChecksAknnAction.setValue(hsneParameters.getNumChecksAKNN());
    _useOutOfCoreComputationAction.setChecked(hsneParameters.useOutOfCoreComputation());
    
    setText("HSNE (advanced)");

    const auto updateNumWalksForLandmarkSelectionAction = [this]() -> void {
        _hsneSettingsAction.getHsneParameters().setNumWalksForLandmarkSelection(_numWalksForLandmarkSelectionAction.getValue());
    };

    const auto updateNumWalksForLandmarkSelectionThreshold = [this]() -> void {
        _hsneSettingsAction.getHsneParameters().setNumWalksForLandmarkSelectionThreshold(_numWalksForLandmarkSelectionThresholdAction.getValue());
    };

    const auto updateRandomWalkLength = [this]() -> void {
        _hsneSettingsAction.getHsneParameters().setRandomWalkLength(_randomWalkLengthAction.getValue());
    };

    const auto updateNumWalksForAreaOfInfluence = [this]() -> void {
        _hsneSettingsAction.getHsneParameters().setNumWalksForAreaOfInfluence(_numWalksForAreaOfInfluenceAction.getValue());
    };

    const auto updateMinWalksRequired = [this]() -> void {
        _hsneSettingsAction.getHsneParameters().setMinWalksRequired(_minWalksRequiredAction.getValue());
    };

    const auto updateNumChecksAknn = [this]() -> void {
        _hsneSettingsAction.getHsneParameters().setNumChecksAKNN(_numChecksAknnAction.getValue());
    };

    const auto updateUseOutOfCoreComputation = [this]() -> void {
    };

    connect(&_numWalksForLandmarkSelectionAction, &IntegralAction::valueChanged, this, [this, updateNumWalksForLandmarkSelectionAction]() {
        updateNumWalksForLandmarkSelectionAction();
    });

    connect(&_numWalksForLandmarkSelectionThresholdAction, &DecimalAction::valueChanged, this, [this, updateNumWalksForLandmarkSelectionThreshold]() {
        updateNumWalksForLandmarkSelectionThreshold();
    });

    connect(&_randomWalkLengthAction, &IntegralAction::valueChanged, this, [this, updateRandomWalkLength]() {
        updateRandomWalkLength();
    });

    connect(&_numWalksForAreaOfInfluenceAction, &IntegralAction::valueChanged, this, [this, updateNumWalksForAreaOfInfluence]() {
        updateNumWalksForAreaOfInfluence();
    });

    connect(&_minWalksRequiredAction, &IntegralAction::valueChanged, this, [this, updateMinWalksRequired]() {
        updateMinWalksRequired();
    });

    connect(&_numChecksAknnAction, &IntegralAction::valueChanged, this, [this, updateNumChecksAknn]() {
        updateNumChecksAknn();
    });

    connect(&_useOutOfCoreComputationAction, &ToggleAction::toggled, this, [this, updateUseOutOfCoreComputation]() {
        updateUseOutOfCoreComputation();
    });

    updateNumWalksForLandmarkSelectionAction();
    updateNumWalksForLandmarkSelectionThreshold();
    updateRandomWalkLength();
    updateNumWalksForAreaOfInfluence();
    updateMinWalksRequired();
    updateNumChecksAknn();
    updateUseOutOfCoreComputation();
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
        auto toggleAction = dynamic_cast<ToggleAction*>(widgetAction);

        if (toggleAction) {
            layout->addWidget(toggleAction->createCheckBoxWidget(this), numRows, 1);
        }
        else {
            layout->addWidget(widgetAction->createLabelWidget(this), numRows, 0);
            layout->addWidget(widgetAction->createWidget(this), numRows, 1);
        }
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