#include "DimensionsSettingsAction.h"
#include "TsneAnalysisPlugin.h"
#include "Application.h"

#include <QLabel>
#include <QTreeView>

using namespace hdps;
using namespace hdps::gui;

DimensionsSettingsAction::DimensionsSettingsAction(TsneAnalysisPlugin* tsneAnalysisPlugin) :
	WidgetActionGroup(tsneAnalysisPlugin),
	_tsneAnalysisPlugin(tsneAnalysisPlugin),
	_selectionHolder(),
	_selectionItemModel(new DimensionSelectionItemModel(_selectionHolder)),
	_selectionProxyModel(new DimensionSelectionProxyModel(_selectionHolder)),
	_nameMatchesAction(this, "Name matches"),
	_showOnlySelectedDimensionsAction(this, "Show only selected dimensions"),
	_applyExclusionListAction(this, "Apply exclusion list"),
	_ignoreZeroValuesAction(this, "Ignore zero values"),
	_selectionThresholdAction(this, "Selection threshold"),
	_summaryAction(this, "Summary"),
	_computeStatisticsAction(this, "Compute statistics"),
	_selectVisibleAction(this, "Select visible"),
	_selectNonVisibleAction(this, "Select non-visible"),
	_loadSelectionAction(this, "Load selection"),
	_saveSelectionAction(this, "Save selection"),
	_loadExclusionAction(this, "Load exclusion")
{
	setText("Dimensions");


	_summaryAction.setEnabled(false);
}

QMenu* DimensionsSettingsAction::getContextMenu()
{
    auto menu = new QMenu(text());
    return menu;
}

DimensionsSettingsAction::Widget::Widget(QWidget* parent, DimensionsSettingsAction* dimensionsSettingsAction, const Widget::State& state) :
    WidgetAction::Widget(parent, dimensionsSettingsAction, state)
{
    auto layout = new QVBoxLayout();

	auto nameMatchesLayout = new QHBoxLayout();

	nameMatchesLayout->addWidget(dimensionsSettingsAction->_nameMatchesAction.createLabelWidget(this));
	nameMatchesLayout->addWidget(dimensionsSettingsAction->_nameMatchesAction.createWidget(this));

	layout->addLayout(nameMatchesLayout);
	
	layout->addWidget(dimensionsSettingsAction->_showOnlySelectedDimensionsAction.createWidget(this));
	layout->addWidget(dimensionsSettingsAction->_applyExclusionListAction.createWidget(this));
	layout->addWidget(dimensionsSettingsAction->_ignoreZeroValuesAction.createWidget(this));

	auto selectionThresholdLayout = new QHBoxLayout();

	selectionThresholdLayout->addWidget(new QLabel("more"));
	selectionThresholdLayout->addWidget(dimensionsSettingsAction->_selectionThresholdAction.createSliderWidget(this));
	selectionThresholdLayout->addWidget(new QLabel("less"));

	layout->addLayout(selectionThresholdLayout);
	
	auto treeView = new QTreeView();

	layout->addWidget(treeView);
	layout->addWidget(dimensionsSettingsAction->_summaryAction.createWidget(this));
	layout->addWidget(dimensionsSettingsAction->_computeStatisticsAction.createWidget(this));

	auto selectLayout = new QHBoxLayout();

	selectLayout->addWidget(dimensionsSettingsAction->_selectVisibleAction.createWidget(this));
	selectLayout->addWidget(dimensionsSettingsAction->_selectNonVisibleAction.createWidget(this));

	layout->addLayout(selectLayout);

	auto fileLayout = new QHBoxLayout();

	fileLayout->addWidget(dimensionsSettingsAction->_loadSelectionAction.createWidget(this));
	fileLayout->addWidget(dimensionsSettingsAction->_loadExclusionAction.createWidget(this));
	fileLayout->addWidget(dimensionsSettingsAction->_saveSelectionAction.createWidget(this));

	layout->addLayout(fileLayout);

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