#include "GeneralHsneSettingsAction.h"
#include "HsneAnalysisPlugin.h"

using namespace hdps::gui;

GeneralHsneSettingsAction::GeneralHsneSettingsAction(HsneSettingsAction& hsneSettingsAction) :
    WidgetActionGroup(&hsneSettingsAction, true),
    _hsneSettingsAction(hsneSettingsAction),
    _knnTypeAction(this, "KNN Type", QStringList({ "FLANN", "HNSW", "ANNOY" }), "FLANN", "FLANN"),
    _seedAction(this, "Random seed", -1, 1000, -1, -1),
    _useMonteCarloSamplingAction(this, "Use Monte Carlo sampling")
{
    setText("HSNE (general)");

    _useMonteCarloSamplingAction.setChecked(hsneSettingsAction.getHsneParameters().useMonteCarloSampling());

    const auto updateKnnAlgorithm = [this]() -> void {
        _hsneSettingsAction.getHsneParameters().setKnnLibrary(static_cast<hdi::dr::knn_library>(_knnTypeAction.getCurrentIndex()));
    };

    const auto updateSeed = [this]() -> void {
        _hsneSettingsAction.getHsneParameters().setSeed(_seedAction.getValue());
    };

    const auto updateUseMonteCarloSampling = [this]() -> void {
    };

    connect(&_knnTypeAction, &OptionAction::currentIndexChanged, this, [this, updateKnnAlgorithm]() {
        updateKnnAlgorithm();
    });

    connect(&_seedAction, &IntegralAction::valueChanged, this, [this, updateSeed]() {
        updateSeed();
    });

    connect(&_useMonteCarloSamplingAction, &ToggleAction::toggled, this, [this, updateUseMonteCarloSampling]() {
        updateUseMonteCarloSampling();
    });

    updateKnnAlgorithm();
    updateSeed();
    updateUseMonteCarloSampling();
}

QMenu* GeneralHsneSettingsAction::getContextMenu()
{
    return nullptr;
}

GeneralHsneSettingsAction::Widget::Widget(QWidget* parent, GeneralHsneSettingsAction* generalHsneSettingsAction, const Widget::State& state) :
    WidgetAction::Widget(parent, generalHsneSettingsAction, state)
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

    addActionToLayout(&generalHsneSettingsAction->_knnTypeAction);
    addActionToLayout(&generalHsneSettingsAction->_seedAction);
    addActionToLayout(&generalHsneSettingsAction->_useMonteCarloSamplingAction);
    
    auto& hsneSettingsAction = generalHsneSettingsAction->getHsneSettingsAction();

    auto startPushButton    = dynamic_cast<TriggerAction::Widget*>(hsneSettingsAction.getStartComputationAction().createWidget(this));
    auto continuePushButton = dynamic_cast<TriggerAction::Widget*>(hsneSettingsAction.getContinueComputationAction().createWidget(this));
    auto stopPushButton     = dynamic_cast<TriggerAction::Widget*>(hsneSettingsAction.getStopComputationAction().createWidget(this));

    auto computeLayout = new QHBoxLayout();

    computeLayout->addWidget(startPushButton);
    computeLayout->addWidget(continuePushButton);
    computeLayout->addWidget(stopPushButton);

    layout->addLayout(computeLayout, layout->rowCount(), 1, 1, 2);

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
