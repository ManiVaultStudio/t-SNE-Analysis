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
    _numChecksAknnAction(this, "No. KNN checks"),
    _useOutOfCoreComputationAction(this, "Use out-of-core computation")
{
    setText("Advanced HSNE");

    const auto& hsneParameters = hsneSettingsAction.getHsneParameters();

    _numWalksForLandmarkSelectionAction.initialize(1, 1000, hsneParameters.getNumWalksForLandmarkSelection(), hsneParameters.getNumWalksForLandmarkSelection());
    _numWalksForLandmarkSelectionThresholdAction.initialize(1, 1000, hsneParameters.getNumWalksForLandmarkSelectionThreshold(), hsneParameters.getNumWalksForLandmarkSelectionThreshold());
    _randomWalkLengthAction.initialize(1, 100, hsneParameters.getRandomWalkLength(), hsneParameters.getRandomWalkLength());
    _numWalksForAreaOfInfluenceAction.initialize(1, 100, hsneParameters.getNumWalksForAreaOfInfluence(), hsneParameters.getNumWalksForAreaOfInfluence());
    _minWalksRequiredAction.initialize(1, 100, hsneParameters.getMinWalksRequired(), hsneParameters.getMinWalksRequired());
    _numChecksAknnAction.initialize(0, 1024, hsneParameters.getNumChecksAKNN(), hsneParameters.getNumChecksAKNN());
    _useOutOfCoreComputationAction.initialize(hsneParameters.useOutOfCoreComputation(), hsneParameters.useOutOfCoreComputation());
    
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
        _hsneSettingsAction.getHsneParameters().useOutOfCoreComputation(_useOutOfCoreComputationAction.isChecked());
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

void AdvancedHsneSettingsAction::setReadOnly(const bool& readOnly)
{
    const auto enabled = !readOnly;

    _numWalksForLandmarkSelectionAction.setEnabled(enabled);
    _numWalksForLandmarkSelectionThresholdAction.setEnabled(enabled);
    _randomWalkLengthAction.setEnabled(enabled);
    _numWalksForAreaOfInfluenceAction.setEnabled(enabled);
    _minWalksRequiredAction.setEnabled(enabled);
    _numChecksAknnAction.setEnabled(enabled);
    _useOutOfCoreComputationAction.setEnabled(enabled);
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