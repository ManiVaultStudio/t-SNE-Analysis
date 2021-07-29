#include "AdvancedSettingsAction.h"
#include "Application.h"
#include "TsneAnalysisPlugin.h"

#include <QLabel>

using namespace hdps::gui;

AdvancedSettingsAction::AdvancedSettingsAction(TsneAnalysisPlugin* tsneAnalysisPlugin) :
	WidgetActionGroup(tsneAnalysisPlugin),
	_tsneAnalysisPlugin(tsneAnalysisPlugin),
	_exaggerationAction(this, "Exaggeration"),
	_exponentialDecayAction(this, "Exponential decay"),
	_numTreesAction(this, "Number of trees"),
	_numChecksAction(this, "Number of checks"),
	_resetAction(this, "Reset all")
{
	setText("Advanced");

	_exaggerationAction.setRange(1, 10000);
	_exponentialDecayAction.setRange(1, 10000);
	_numTreesAction.setRange(1, 10000);
	_numChecksAction.setRange(1, 10000);

	_exaggerationAction.setValue(250);
	_exponentialDecayAction.setValue(70);
	_numTreesAction.setValue(4);
	_numChecksAction.setValue(1024);

	_exaggerationAction.setDefaultValue(250);
	_exponentialDecayAction.setDefaultValue(70);
	_numTreesAction.setDefaultValue(4);
	_numChecksAction.setDefaultValue(1024);

	const auto updateExaggeration = [this]() -> void {
        _tsneAnalysisPlugin->getTsneParameters().setExaggerationIter(_exaggerationAction.getValue());
	};

	const auto updateExponentialDecay = [this]() -> void {
        _tsneAnalysisPlugin->getTsneParameters().setExponentialDecayIter(_exponentialDecayAction.getValue());
	};

	const auto updateNumTrees = [this]() -> void {
        _tsneAnalysisPlugin->getTsneParameters().setNumTrees(_numTreesAction.getValue());
	};

	const auto updateNumChecks = [this]() -> void {
        _tsneAnalysisPlugin->getTsneParameters().setNumChecks(_numChecksAction.getValue());
	};

	const auto canReset = [this]() -> bool {
		if (_exaggerationAction.canReset())
			return true;

		if (_exponentialDecayAction.canReset())
			return true;

		if (_numTreesAction.canReset())
			return true;

		if (_numChecksAction.canReset())
			return true;

		return false;
	};

	const auto updateReset = [this, canReset]() -> void {
		_resetAction.setEnabled(canReset());
	};

	connect(&_exaggerationAction, &IntegralAction::valueChanged, this, [this, updateExaggeration, updateReset](const std::int32_t& value) {
		updateExaggeration();
		updateReset();
	});

	connect(&_exponentialDecayAction, &IntegralAction::valueChanged, this, [this, updateExponentialDecay, updateReset](const std::int32_t& value) {
		updateExponentialDecay();
		updateReset();
	});

	connect(&_numTreesAction, &IntegralAction::valueChanged, this, [this, updateNumTrees, updateReset](const std::int32_t& value) {
		updateNumTrees();
		updateReset();
	});

	connect(&_numChecksAction, &IntegralAction::valueChanged, this, [this, updateNumChecks, updateReset](const std::int32_t& value) {
		updateNumChecks();
		updateReset();
	});

	connect(&_resetAction, &TriggerAction::triggered, this, [this](const std::int32_t& value) {
		_exaggerationAction.reset();
		_exponentialDecayAction.reset();
		_numTreesAction.reset();
		_numChecksAction.reset();
	});

	const auto enableControls = [this]() -> void {
		const auto enable = !_tsneAnalysisPlugin->getGeneralSettingsAction().getRunningAction().isChecked();

		_exaggerationAction.setEnabled(enable);
		_exponentialDecayAction.setEnabled(enable);
		_numTreesAction.setEnabled(enable);
		_numChecksAction.setEnabled(enable);
	};

	connect(&_tsneAnalysisPlugin->getGeneralSettingsAction().getRunningAction(), &ToggleAction::toggled, this, [this, enableControls](bool toggled) {
		enableControls();
	});

	updateExaggeration();
	updateExponentialDecay();
	updateNumTrees();
	updateNumChecks();
	updateReset();
	enableControls();

	_resetAction.setEnabled(false);
}

QMenu* AdvancedSettingsAction::getContextMenu()
{
    auto menu = new QMenu(text());

	const auto addSetting = [this, menu](WidgetAction& widgetAction) -> void {
		auto settingMenu = new QMenu(widgetAction.text());
		settingMenu->addAction(&widgetAction);
		menu->addMenu(settingMenu);
	};

	addSetting(_exaggerationAction);
	addSetting(_exponentialDecayAction);
	addSetting(_numTreesAction);
	addSetting(_numChecksAction);

	menu->addSeparator();

	menu->addAction(&_resetAction);

    return menu;
}

AdvancedSettingsAction::Widget::Widget(QWidget* parent, AdvancedSettingsAction* advancedSettingsAction, const Widget::State& state) :
    WidgetAction::Widget(parent, advancedSettingsAction, state)
{
    auto layout = new QGridLayout();

	const auto addIntegralActionToLayout = [this, layout](IntegralAction& integralAction) -> void {
		const auto numRows = layout->rowCount();

		layout->addWidget(integralAction.createLabelWidget(this), numRows, 0);
		layout->addWidget(integralAction.createWidget(this), numRows, 1);
	};

	addIntegralActionToLayout(advancedSettingsAction->_exaggerationAction);
	addIntegralActionToLayout(advancedSettingsAction->_exponentialDecayAction);
	addIntegralActionToLayout(advancedSettingsAction->_numTreesAction);
	addIntegralActionToLayout(advancedSettingsAction->_numChecksAction);

	layout->addWidget(advancedSettingsAction->_resetAction.createWidget(this), layout->rowCount(), 0, 1, 2);

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