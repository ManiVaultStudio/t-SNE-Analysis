#include "SettingsAction.h"
#include "Application.h"
#include "TsneAnalysisPlugin.h"

#include <QLabel>

using namespace hdps::gui;

SettingsAction::SettingsAction(TsneAnalysisPlugin* tsneAnalysisPlugin) :
    WidgetAction(tsneAnalysisPlugin),
	_knnTypeAction(this, "KNN Type"),
	_distanceMetricAction(this, "Distance metric"),
	_numIterationsAction(this, "Number of iterations"),
	_perplexityAction(this, "Perplexity"),
	_exaggerationAction(this, "Exaggeration"),
	_exponentialDecayAction(this, "Exponential decay"),
	_numTreesAction(this, "Number of trees"),
	_numChecksAction(this, "Number of checks"),
	_thetaAction(this, "Theta"),
	_startComputationAction(this, "Start computation"),
	_stopComputationAction(this, "Stop computation")
{
	_knnTypeAction.setOptions(QStringList() << "FLANN" << "HNSW" << "ANNOY");
	_distanceMetricAction.setOptions(QStringList() << "Euclidean" << "Cosine" << "Inner Product" << "Manhattan" << "Hamming" << "Dot");
	_numIterationsAction.setRange(1, 10000);
	_perplexityAction.setRange(2, 50);
	_exaggerationAction.setRange(1, 10000);
	_numTreesAction.setRange(1, 10000);
	_numChecksAction.setRange(1, 10000);

	setText("Settings");
}

QMenu* SettingsAction::getContextMenu()
{
    auto menu = new QMenu("TSNE Analysis");

	auto settingsMenu = new QMenu("Settings");

	settingsMenu->addAction(&_knnTypeAction);
	settingsMenu->addAction(&_distanceMetricAction);
	settingsMenu->addAction(&_numIterationsAction);
	settingsMenu->addAction(&_exaggerationAction);
	settingsMenu->addAction(&_exponentialDecayAction);
	settingsMenu->addAction(&_numTreesAction);
	settingsMenu->addAction(&_numChecksAction);
	settingsMenu->addAction(&_thetaAction);

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
		layout->addWidget(optionAction.createWidget(this), numRows, 1, 1, 2);
	};

	const auto addIntegralActionToLayout = [this, layout](IntegralAction& integralAction) -> void {
		const auto numRows = layout->rowCount();

		layout->addWidget(new QLabel(integralAction.text()), numRows, 0);
		layout->addWidget(integralAction.createSpinBoxWidget(this), numRows, 1);
		layout->addWidget(integralAction.createSliderWidget(this), numRows, 2);
	};

	addOptionActionToLayout(settingsAction->_knnTypeAction);
	addOptionActionToLayout(settingsAction->_distanceMetricAction);
	addIntegralActionToLayout(settingsAction->_numIterationsAction);
	addIntegralActionToLayout(settingsAction->_perplexityAction);
	addIntegralActionToLayout(settingsAction->_exaggerationAction);
	addIntegralActionToLayout(settingsAction->_exponentialDecayAction);
	addIntegralActionToLayout(settingsAction->_numTreesAction);
	addIntegralActionToLayout(settingsAction->_numChecksAction);
	addIntegralActionToLayout(settingsAction->_thetaAction);

    layout->addWidget(settingsAction->_startComputationAction.createWidget(this), layout->rowCount(), 0, 1, 3);
    layout->addWidget(settingsAction->_stopComputationAction.createWidget(this), layout->rowCount(), 0, 1, 3);

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