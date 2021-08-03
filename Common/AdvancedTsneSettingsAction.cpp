#include "AdvancedTsneSettingsAction.h"
#include "TsneSettingsAction.h"

#include <QTableView>

using namespace hdps::gui;

AdvancedTsneSettingsAction::AdvancedTsneSettingsAction(TsneSettingsAction& tsneSettingsAction) :
    WidgetActionGroup(&tsneSettingsAction),
    _tsneSettingsAction(tsneSettingsAction),
	_exaggerationAction(this, "Exaggeration"),
	_exponentialDecayAction(this, "Exponential decay"),
	_numTreesAction(this, "Number of trees"),
	_numChecksAction(this, "Number of checks"),
	_resetAction(this, "Reset all")
{
	setText("Advanced TSNE");

    const auto& tsneParameters = _tsneSettingsAction.getTsneParameters();

    _exaggerationAction.initialize(1, 10000, 250, 250);
    _exponentialDecayAction.initialize(1, 10000, 70, 70);
    _numTreesAction.initialize(1, 10000, 4, 4);
    _numChecksAction.initialize(1, 10000, 1024, 1024);

	const auto updateExaggeration = [this]() -> void {
        _tsneSettingsAction.getTsneParameters().setExaggerationIter(_exaggerationAction.getValue());
	};

	const auto updateExponentialDecay = [this]() -> void {
        _tsneSettingsAction.getTsneParameters().setExponentialDecayIter(_exponentialDecayAction.getValue());
	};

	const auto updateNumTrees = [this]() -> void {
        _tsneSettingsAction.getTsneParameters().setNumTrees(_numTreesAction.getValue());
	};

	const auto updateNumChecks = [this]() -> void {
        _tsneSettingsAction.getTsneParameters().setNumChecks(_numChecksAction.getValue());
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

	updateExaggeration();
	updateExponentialDecay();
	updateNumTrees();
	updateNumChecks();
	updateReset();
}

void AdvancedTsneSettingsAction::setReadOnly(const bool& readOnly)
{
    const auto enable = !readOnly;

    _exaggerationAction.setEnabled(enable);
    _exponentialDecayAction.setEnabled(enable);
    _numTreesAction.setEnabled(enable);
    _numChecksAction.setEnabled(enable);
    _resetAction.setEnabled(enable);
}

AdvancedTsneSettingsAction::Widget::Widget(QWidget* parent, AdvancedTsneSettingsAction* advancedTsneSettingsAction, const Widget::State& state) :
    WidgetAction::Widget(parent, advancedTsneSettingsAction, state)
{
    auto layout = new QGridLayout();

	const auto addIntegralActionToLayout = [this, layout](IntegralAction& integralAction) -> void {
		const auto numRows = layout->rowCount();

		layout->addWidget(integralAction.createLabelWidget(this), numRows, 0);
		layout->addWidget(integralAction.createWidget(this), numRows, 1);
	};

	addIntegralActionToLayout(advancedTsneSettingsAction->_exaggerationAction);
	addIntegralActionToLayout(advancedTsneSettingsAction->_exponentialDecayAction);
	addIntegralActionToLayout(advancedTsneSettingsAction->_numTreesAction);
	addIntegralActionToLayout(advancedTsneSettingsAction->_numChecksAction);

	layout->addWidget(advancedTsneSettingsAction->_resetAction.createWidget(this), layout->rowCount(), 0, 1, 2);

    layout->update();

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