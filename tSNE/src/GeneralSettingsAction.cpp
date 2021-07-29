#include "GeneralSettingsAction.h"
#include "Application.h"
#include "TsneAnalysisPlugin.h"

#include <QLabel>

using namespace hdps::gui;

GeneralSettingsAction::GeneralSettingsAction(TsneAnalysisPlugin* tsneAnalysisPlugin) :
	WidgetActionGroup(tsneAnalysisPlugin, true),
	_tsneAnalysisPlugin(tsneAnalysisPlugin),
	_knnTypeAction(this, "KNN Type"),
	_distanceMetricAction(this, "Distance metric"),
	_numIterationsAction(this, "Number of iterations"),
	_perplexityAction(this, "Perplexity"),
	_resetAction(this, "Reset all"),
	_startComputationAction(this, "Start"),
	_continueComputationAction(this, "Continue"),
	_stopComputationAction(this, "Stop"),
	_runningAction(this, "Running")
{
	setText("General");

	_knnTypeAction.setOptions(QStringList() << "FLANN" << "HNSW" << "ANNOY");
	_distanceMetricAction.setOptions(QStringList() << "Euclidean" << "Cosine" << "Inner Product" << "Manhattan" << "Hamming" << "Dot");
	_numIterationsAction.setRange(1, 10000);
	_perplexityAction.setRange(2, 50);

	_knnTypeAction.setCurrentText("FLANN");
	_distanceMetricAction.setCurrentText("Euclidean");
	_numIterationsAction.setValue(1000);
	_perplexityAction.setValue(30);

	_knnTypeAction.setDefaultText("FLANN");
	_distanceMetricAction.setDefaultText("Euclidean");
	_numIterationsAction.setDefaultValue(1000);
	_perplexityAction.setDefaultValue(30);

	_resetAction.setEnabled(false);

	const auto updateKnnAlgorithm = [this]() -> void {
		_tsneAnalysisPlugin->getTsneParameters().setKnnAlgorithm(static_cast<hdi::dr::knn_library>(_knnTypeAction.getCurrentIndex()));
	};

	const auto updateDistanceMetric = [this]() -> void {
        _tsneAnalysisPlugin->getTsneParameters().setKnnDistanceMetric(static_cast<hdi::dr::knn_distance_metric>(_distanceMetricAction.getCurrentIndex()));
	};

	const auto updateNumIterations = [this]() -> void {
        _tsneAnalysisPlugin->getTsneParameters().setNumIterations(_numIterationsAction.getValue());
	};

	const auto updatePerplexity = [this]() -> void {
        _tsneAnalysisPlugin->getTsneParameters().setPerplexity(_perplexityAction.getValue());
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

		return false;
	};

	/*
	const auto updateComputation = [this]() -> void {
		const auto isRunning = _runningAction.isChecked();

		_computationAction.setText(isRunning ? "Stop computation" : "Start computation");
		_computationAction.setText("");
		_computationAction.setIcon(isRunning ? hdps::Application::getIconFont("FontAwesome").getIcon("stop") : hdps::Application::getIconFont("FontAwesome").getIcon("play"));
	};
	*/

	const auto updateReset = [this, canReset]() -> void {
		_resetAction.setEnabled(canReset());
	};

	const auto enableControls = [this, canReset]() -> void {
		const auto isRunning	= _runningAction.isChecked();
		const auto enable		= !isRunning;

		_knnTypeAction.setEnabled(enable);
		_distanceMetricAction.setEnabled(enable);
		_numIterationsAction.setEnabled(enable);
		_perplexityAction.setEnabled(enable);
		_resetAction.setEnabled(enable && canReset());
		//_startComputationAction.setEnabled(enable);
		//_continueComputationAction.setEnabled(enable);
		//_stopComputationAction.setEnabled(isRunning);
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

	connect(&_startComputationAction, &TriggerAction::triggered, this, [this]() {
		/*
		QSignalBlocker blocker(&_computationAction);

		_computationAction.setChecked(true);
		_computationAction.setText("");
		_computationAction.setIcon(hdps::Application::getIconFont("FontAwesome").getIcon("stop"));
		*/
	});

	connect(&_continueComputationAction, &TriggerAction::triggered, this, [this]() {
		/*
		QSignalBlocker blocker(&_computationAction);

		_computationAction.setChecked(true);
		_computationAction.setText("");
		_computationAction.setIcon(hdps::Application::getIconFont("FontAwesome").getIcon("stop"));
		*/
	});

	connect(&_stopComputationAction, &TriggerAction::triggered, this, [this]() {
		/*
		QSignalBlocker blocker(&_computationAction);

		_computationAction.setChecked(false);
		_computationAction.setText("");
		_computationAction.setIcon(hdps::Application::getIconFont("FontAwesome").getIcon("play"));
		*/
	});

	connect(&_runningAction, &TriggerAction::toggled, this, [this, enableControls](bool toggled) {
		enableControls();
	});

	connect(&_resetAction, &TriggerAction::triggered, this, [this, enableControls](const std::int32_t& value) {
		_knnTypeAction.reset();
		_distanceMetricAction.reset();
		_numIterationsAction.reset();
		_perplexityAction.reset();
	});

	updateKnnAlgorithm();
	updateDistanceMetric();
	updateNumIterations();
	updatePerplexity();
	updateReset();
	enableControls();
}

QMenu* GeneralSettingsAction::getContextMenu()
{
    auto menu = new QMenu(text());

	menu->addAction(&_startComputationAction);
	menu->addAction(&_stopComputationAction);

	menu->addSeparator();

	const auto addSetting = [this, menu](WidgetAction& widgetAction) -> void {
		auto settingMenu = new QMenu(widgetAction.text());
		settingMenu->addAction(&widgetAction);
		menu->addMenu(settingMenu);
	};

	addSetting(_knnTypeAction);
	addSetting(_distanceMetricAction);
	addSetting(_numIterationsAction);

	menu->addSeparator();

	menu->addAction(&_resetAction);

    return menu;
}

GeneralSettingsAction::Widget::Widget(QWidget* parent, GeneralSettingsAction* generalSettingsAction, const Widget::State& state) :
    WidgetAction::Widget(parent, generalSettingsAction, state)
{
    auto layout = new QGridLayout();

	const auto addOptionActionToLayout = [this, layout](OptionAction& optionAction) -> void {
		const auto numRows = layout->rowCount();

		layout->addWidget(optionAction.createLabelWidget(this), numRows, 0);
		layout->addWidget(optionAction.createWidget(this), numRows, 1);
	};

	const auto addIntegralActionToLayout = [this, layout](IntegralAction& integralAction) -> void {
		const auto numRows = layout->rowCount();

		layout->addWidget(integralAction.createLabelWidget(this), numRows, 0);
		layout->addWidget(integralAction.createWidget(this), numRows, 1);
	};

	addOptionActionToLayout(generalSettingsAction->_knnTypeAction);
	addOptionActionToLayout(generalSettingsAction->_distanceMetricAction);
	addIntegralActionToLayout(generalSettingsAction->_numIterationsAction);
	addIntegralActionToLayout(generalSettingsAction->_perplexityAction);

	layout->addWidget(generalSettingsAction->_resetAction.createWidget(this), layout->rowCount(), 1, 1, 2);
	
	auto computeLayout = new QHBoxLayout();

	computeLayout->addWidget(generalSettingsAction->_startComputationAction.createWidget(this));
	computeLayout->addWidget(generalSettingsAction->_continueComputationAction.createWidget(this));
	computeLayout->addWidget(generalSettingsAction->_stopComputationAction.createWidget(this));

	layout->addLayout(computeLayout, layout->rowCount(), 1, 1, 2);

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