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
	_startComputationAction(this, "Start computation"),
	_stopComputationAction(this, "Stop computation"),
	_computationAction(this, "Computation")
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

	const auto updateComputation = [this]() -> void {
		_computationAction.setText(_computationAction.isChecked() ? "Stop computation" : "Start computation");
		_computationAction.setText("");
		_computationAction.setIcon(_computationAction.isChecked() ? hdps::Application::getIconFont("FontAwesome").getIcon("stop") : hdps::Application::getIconFont("FontAwesome").getIcon("play"));
	};

	const auto updateReset = [this, canReset]() -> void {
		_resetAction.setEnabled(canReset());
	};

	const auto enableControls = [this](const bool& enabled) -> void {
		_knnTypeAction.setEnabled(enabled);
		_distanceMetricAction.setEnabled(enabled);
		_numIterationsAction.setEnabled(enabled);
		_perplexityAction.setEnabled(enabled);
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

	connect(&_startComputationAction, &TriggerAction::triggered, this, [this, enableControls, updateComputation](const std::int32_t& value) {
		enableControls(false);
		updateComputation();

		QSignalBlocker blocker(&_computationAction);

		_computationAction.setChecked(true);
		_computationAction.setText("");
		_computationAction.setIcon(hdps::Application::getIconFont("FontAwesome").getIcon("stop"));
	});

	connect(&_stopComputationAction, &TriggerAction::triggered, this, [this, enableControls, updateComputation](const std::int32_t& value) {
		enableControls(true);
		updateComputation();

		QSignalBlocker blocker(&_computationAction);

		_computationAction.setChecked(false);
		_computationAction.setText("");
		_computationAction.setIcon(hdps::Application::getIconFont("FontAwesome").getIcon("play"));
	});

	connect(&_computationAction, &ToggleAction::toggled, this, [this, updateComputation](bool toggled) {
		updateComputation();

		if (toggled)
			_startComputationAction.trigger();
		else
			_stopComputationAction.trigger();
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
	updateComputation();
	updateReset();
}

QMenu* GeneralSettingsAction::getContextMenu()
{
    auto menu = new QMenu(text());

	menu->addAction(&_startComputationAction);
	menu->addAction(&_stopComputationAction);

	menu->addSeparator();

	auto settingsMenu = new QMenu("Settings");

	const auto addSetting = [this, settingsMenu](WidgetAction& widgetAction) -> void {
		auto settingMenu = new QMenu(widgetAction.text());
		settingMenu->addAction(&widgetAction);
		settingsMenu->addMenu(settingMenu);
	};

	addSetting(_knnTypeAction);
	addSetting(_distanceMetricAction);
	addSetting(_numIterationsAction);

	settingsMenu->addSeparator();

	settingsMenu->addAction(&_resetAction);

    menu->addMenu(settingsMenu);

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

	layout->addWidget(generalSettingsAction->_resetAction.createWidget(this), layout->rowCount(), 0, 1, 3);
	layout->addWidget(generalSettingsAction->_computationAction.createPushButtonWidget(this), layout->rowCount(), 0, 1, 3);

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