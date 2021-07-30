#pragma once

#include "DimensionSelectionHolder.h"
#include "DimensionSelectionItemModel.h"
#include "DimensionSelectionProxyModel.h"

#include "actions/Actions.h"

class QMenu;
class TsneAnalysisPlugin;
class Points;

class DimensionSelectionAction : public hdps::gui::WidgetActionGroup
{
protected:

    class Widget : public hdps::gui::WidgetActionGroup::Widget {
    public:
        Widget(QWidget* parent, DimensionSelectionAction* dimensionsSettingsAction, const Widget::State& state);
    };

    QWidget* getWidget(QWidget* parent, const Widget::State& state = Widget::State::Standard) override {
        return new Widget(parent, this, state);
    };

public:
	DimensionSelectionAction(TsneAnalysisPlugin* tsneAnalysisPlugin);
	~DimensionSelectionAction();

    QMenu* getContextMenu() { return nullptr; }

	void setDimensions(const std::uint32_t numDimensions, const std::vector<QString>& names);
	std::vector<bool> getEnabledDimensions() const;
	void dataChanged(Points& points);

	hdps::DimensionSelectionProxyModel* getProxyModel();

public:
	void setNameFilter(const QString& nameFilter);
	void setShowOnlySelectedDimensions(const bool& showOnlySelectedDimensions);
	void setApplyExclusionList(const bool& applyExclusionList);
	void setIgnoreZeroValues(const bool& ignoreZeroValues);
	void loadSelectionFromFile(const QString& fileName);
	void loadExclusionFromFile(const QString& fileName);
	void saveSelectionToFile(const QString& fileName);

protected:
	void computeStatistics();
	void updateSlider();
	void updateSummary();

	template <bool selectVisible>
	void selectDimensionsBasedOnVisibility()
	{
		const auto n = _selectionHolder.getNumberOfDimensions();

		if (n > INT_MAX)
		{
			assert(!"Number of dimensions too large -- Qt uses int!");
		}
		else
		{
			for (unsigned i{}; i < n; ++i)
				_selectionHolder.setDimensionEnabled(i, _selectionProxyModel->filterAcceptsRow(i, QModelIndex()) == selectVisible);
			
			const ModelResetter modelResetter(_selectionProxyModel.get());
		}
	}

protected:
	TsneAnalysisPlugin*									_tsneAnalysisPlugin;					/** Pointer to TSNE analysis plugin */
	Points*												_pointData;								/** Pointer to points set */
	hdps::DimensionSelectionHolder						_selectionHolder;						/** Selection holder */
	std::unique_ptr<hdps::DimensionSelectionItemModel>	_selectionItemModel;					/** Selection item model */
	std::unique_ptr<hdps::DimensionSelectionProxyModel>	_selectionProxyModel;					/** Selection proxy model for filtering etc. */
	hdps::gui::StringAction								_nameFilterAction;						/** Name filter action */
	hdps::gui::ToggleAction								_showOnlySelectedDimensionsAction;		/** Show only selected dimensions action */
	hdps::gui::ToggleAction								_applyExclusionListAction;				/** Apply exclusion list action */
	hdps::gui::ToggleAction								_ignoreZeroValuesAction;				/** Ignore zero values for statistics action */
	hdps::gui::IntegralAction							_selectionThresholdAction;				/** Selection threshold action */
	hdps::gui::StringAction								_summaryAction;							/** Summary action */
	hdps::gui::TriggerAction							_computeStatisticsAction;				/** Compute statistics action */
	hdps::gui::TriggerAction							_selectVisibleAction;					/** Select visible dimensions action */
	hdps::gui::TriggerAction							_selectNonVisibleAction;				/** Select non visible dimensions action */
	hdps::gui::TriggerAction							_loadSelectionAction;					/** Load selection action */
	hdps::gui::TriggerAction							_saveSelectionAction;					/** Save selection action */
	hdps::gui::TriggerAction							_loadExclusionAction;					/** Load exclusion action */
	QMetaObject::Connection								_summaryUpdateAwakeConnection;			/** Update summary view when idle */

    friend class Widget;
};