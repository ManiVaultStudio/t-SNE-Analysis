#include "SettingsAction.h"
#include "Application.h"
#include "TsneAnalysisPlugin.h"

#include <QLabel>

using namespace hdps::gui;

SettingsAction::SettingsAction(TsneAnalysisPlugin* tsneAnalysisPlugin) :
    WidgetAction(tsneAnalysisPlugin),
	_tsneAnalysisPlugin(tsneAnalysisPlugin),
	_knnTypeAction(this, "KNN Type"),
	_distanceMetricAction(this, "Distance metric"),
	_numIterationsAction(this, "Number of iterations"),
	_perplexityAction(this, "Perplexity"),
	_exaggerationAction(this, "Exaggeration"),
	_exponentialDecayAction(this, "Exponential decay"),
	_numTreesAction(this, "Number of trees"),
	_numChecksAction(this, "Number of checks"),
	_resetAction(this, "Reset all"),
	_startComputationAction(this, "Start computation"),
	_stopComputationAction(this, "Stop computation")
{
	setText("Settings");

	_knnTypeAction.setOptions(QStringList() << "FLANN" << "HNSW" << "ANNOY");
	_distanceMetricAction.setOptions(QStringList() << "Euclidean" << "Cosine" << "Inner Product" << "Manhattan" << "Hamming" << "Dot");
	_numIterationsAction.setRange(1, 10000);
	_perplexityAction.setRange(2, 50);
	_exaggerationAction.setRange(1, 10000);
	_exponentialDecayAction.setRange(1, 10000);
	_numTreesAction.setRange(1, 10000);
	_numChecksAction.setRange(1, 10000);

	_numIterationsAction.setValue(1000);
	_perplexityAction.setValue(30);
	_exaggerationAction.setValue(250);
	_exponentialDecayAction.setValue(70);
	_numTreesAction.setValue(4);
	_numChecksAction.setValue(1024);

	_numIterationsAction.setDefaultValue(1000);
	_perplexityAction.setDefaultValue(30);
	_exaggerationAction.setDefaultValue(250);
	_exponentialDecayAction.setDefaultValue(70);
	_numTreesAction.setDefaultValue(4);
	_numChecksAction.setDefaultValue(1024);

	const auto updateKnnAlgorithm = [this]() -> void {
		_tsneAnalysisPlugin->_tsne.setKnnAlgorithm(_knnTypeAction.getCurrentIndex());
	};

	const auto updateDistanceMetric = [this]() -> void {
		_tsneAnalysisPlugin->_tsne.setDistanceMetric(_distanceMetricAction.getCurrentIndex());
	};

	const auto updateNumIterations = [this]() -> void {
		_tsneAnalysisPlugin->_tsne.setIterations(_numIterationsAction.getValue());
	};

	const auto updatePerplexity = [this]() -> void {
		_tsneAnalysisPlugin->_tsne.setPerplexity(_perplexityAction.getValue());
	};

	const auto updateExaggeration = [this]() -> void {
		_tsneAnalysisPlugin->_tsne.setExaggerationIter(_exaggerationAction.getValue());
	};

	const auto updateExponentialDecay = [this]() -> void {
		_tsneAnalysisPlugin->_tsne.setExponentialDecayIter(_exponentialDecayAction.getValue());
	};

	const auto updateNumTrees = [this]() -> void {
		_tsneAnalysisPlugin->_tsne.setNumTrees(_numTreesAction.getValue());
	};

	const auto updateNumChecks = [this]() -> void {
		_tsneAnalysisPlugin->_tsne.setNumChecks(_numChecksAction.getValue());
	};

	const auto canReset = [this]() -> bool {
		if (_knnTypeAction.canReset())
			return true;

		if (_distanceMetricAction.canReset())
			return true;

		if (_numIterationsAction.canReset())
			return true;

		if (_perplexityAction.canReset())
			return true;

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

	const auto enableControls = [this](const bool& enabled) -> void {
		_knnTypeAction.setEnabled(enabled);
		_distanceMetricAction.setEnabled(enabled);
		_numIterationsAction.setEnabled(enabled);
		_perplexityAction.setEnabled(enabled);
		_exaggerationAction.setEnabled(enabled);
		_exponentialDecayAction.setEnabled(enabled);
		_numTreesAction.setEnabled(enabled);
		_numChecksAction.setEnabled(enabled);
		_resetAction.setEnabled(enabled);
	};

	connect(&_knnTypeAction, &OptionAction::currentIndexChanged, this, [this, updateDistanceMetric, updateReset](const std::int32_t& currentIndex) {
		updateDistanceMetric();
		updateReset();
	});

	connect(&_distanceMetricAction, &OptionAction::currentIndexChanged, this, [this, updateDistanceMetric, updateReset](const std::int32_t& currentIndex) {
		updateDistanceMetric();
		updateReset();
	});

	connect(&_numIterationsAction, &IntegralAction::valueChanged, this, [this, updateNumIterations, updateReset](const std::int32_t& value) {
		updateNumIterations();
		updateReset();
	});

	connect(&_perplexityAction, &IntegralAction::valueChanged, this, [this, updatePerplexity, updateReset](const std::int32_t& value) {
		updatePerplexity();
		updateReset();
	});

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

	connect(&_startComputationAction, &TriggerAction::triggered, this, [this, enableControls](const std::int32_t& value) {
		enableControls(false);
	});

	connect(&_stopComputationAction, &TriggerAction::triggered, this, [this, enableControls](const std::int32_t& value) {
		enableControls(true);
	});

	connect(&_resetAction, &TriggerAction::triggered, this, [this, enableControls](const std::int32_t& value) {
		_knnTypeAction.reset();
		_distanceMetricAction.reset();
		_numIterationsAction.reset();
		_perplexityAction.reset();
		_exaggerationAction.reset();
		_exponentialDecayAction.reset();
		_numTreesAction.reset();
		_numChecksAction.reset();
	});

	updateKnnAlgorithm();
	updateDistanceMetric();
	updateNumIterations();
	updatePerplexity();
	updateExaggeration();
	updateExponentialDecay();
	updateNumTrees();
	updateNumChecks();
}

QMenu* SettingsAction::getContextMenu()
{
    auto menu = new QMenu("TSNE Analysis");

	auto settingsMenu = new QMenu("Settings");

	/*
	settingsMenu->addAction(&_knnTypeAction);
	settingsMenu->addAction(&_distanceMetricAction);
	settingsMenu->addAction(&_numIterationsAction);
	settingsMenu->addAction(&_exaggerationAction);
	settingsMenu->addAction(&_exponentialDecayAction);
	settingsMenu->addAction(&_numTreesAction);
	settingsMenu->addAction(&_numChecksAction);
	settingsMenu->addAction(&_thetaAction);
	*/

    menu->addMenu(settingsMenu);

    menu->addAction(&_startComputationAction);
    menu->addAction(&_stopComputationAction);

    return menu;
}

SettingsAction::Widget::Widget(QWidget* parent, SettingsAction* settingsAction, const Widget::State& state) :
    WidgetAction::Widget(parent, settingsAction, state)
{
    auto layout = new QGridLayout();

	const auto addOptionActionToLayout = [this, layout](OptionAction& optionAction) -> void {
		const auto numRows = layout->rowCount();

		layout->addWidget(new QLabel(optionAction.text()), numRows, 0);
		layout->addWidget(optionAction.createWidget(this), numRows, 1);
	};

	const auto addIntegralActionToLayout = [this, layout](IntegralAction& integralAction) -> void {
		const auto numRows = layout->rowCount();

		layout->addWidget(new QLabel(integralAction.text()), numRows, 0);
		layout->addWidget(integralAction.createWidget(this), numRows, 1);
	};

	addOptionActionToLayout(settingsAction->_knnTypeAction);
	addOptionActionToLayout(settingsAction->_distanceMetricAction);
	addIntegralActionToLayout(settingsAction->_numIterationsAction);
	addIntegralActionToLayout(settingsAction->_perplexityAction);
	addIntegralActionToLayout(settingsAction->_exaggerationAction);
	addIntegralActionToLayout(settingsAction->_exponentialDecayAction);
	addIntegralActionToLayout(settingsAction->_numTreesAction);
	addIntegralActionToLayout(settingsAction->_numChecksAction);

	layout->addWidget(settingsAction->_resetAction.createWidget(this), layout->rowCount(), 0, 1, 2);
    layout->addWidget(settingsAction->_startComputationAction.createWidget(this), layout->rowCount(), 0, 1, 2);
    layout->addWidget(settingsAction->_stopComputationAction.createWidget(this), layout->rowCount(), 0, 1, 2);

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