#pragma once

#include "DimensionSelectionHolder.h"
#include "DimensionSelectionItemModel.h"
#include "DimensionSelectionProxyModel.h"

#include "actions/Actions.h"

class QMenu;
class TsneAnalysisPlugin;

class DimensionsSettingsAction : public hdps::gui::WidgetActionGroup
{
protected:

    class Widget : public hdps::gui::WidgetActionGroup::Widget {
    public:
        Widget(QWidget* parent, DimensionsSettingsAction* dimensionsSettingsAction, const Widget::State& state);
    };

    QWidget* getWidget(QWidget* parent, const Widget::State& state = Widget::State::Standard) override {
        return new Widget(parent, this, state);
    };

public:
	DimensionsSettingsAction(TsneAnalysisPlugin* tsneAnalysisPlugin);

    QMenu* getContextMenu();

protected:
	TsneAnalysisPlugin*						_tsneAnalysisPlugin;					/** Pointer to TSNE analysis plugin */
	hdps::DimensionSelectionHolder			_selectionHolder;						/** Selection holder */
	hdps::DimensionSelectionItemModel*		_selectionItemModel;					/** Selection item model */
	hdps::DimensionSelectionProxyModel*		_selectionProxyModel;					/** Selection proxy model for filtering etc. */
	hdps::gui::StringAction					_nameMatchesAction;						/** Name filter action */
	hdps::gui::ToggleAction					_showOnlySelectedDimensionsAction;		/** Show only selected dimensions action */
	hdps::gui::ToggleAction					_applyExclusionListAction;				/** Apply exclusion list action */
	hdps::gui::ToggleAction					_ignoreZeroValuesAction;				/** Ignore zero values for statistics action */
	hdps::gui::IntegralAction				_selectionThresholdAction;				/** Selection threshold action */
	hdps::gui::StringAction					_summaryAction;							/** Summary action */
	hdps::gui::TriggerAction				_computeStatisticsAction;				/** Compute statistics action */
	hdps::gui::TriggerAction				_selectVisibleAction;					/** Select visible dimensions action */
	hdps::gui::TriggerAction				_selectNonVisibleAction;				/** Select non visible dimensions action */
	hdps::gui::TriggerAction				_loadSelectionAction;					/** Load selection action */
	hdps::gui::TriggerAction				_saveSelectionAction;					/** Save selection action */
	hdps::gui::TriggerAction				_loadExclusionAction;					/** Load exclusion action */

    friend class Widget;
};