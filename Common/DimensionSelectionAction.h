#pragma once

#include "DimensionSelectionHolder.h"
#include "DimensionSelectionItemModel.h"
#include "DimensionSelectionProxyModel.h"

#include "actions/Actions.h"

class QMenu;
class Points;

/**
 * Dimension selection action class
 *
 * Action class for point data dimension selection
 *
 * @author Thomas Kroes
 */
class DimensionSelectionAction : public hdps::gui::WidgetActionGroup
{
protected:

    /** Widget class for dimension selection action */
    class Widget : public hdps::gui::WidgetActionGroup::Widget {
    public:

        /**
         * Constructor
         * @param parent Pointer to parent widget
         * @param dimensionSelectionAction Pointer to dimension selection action
         * @param state State of the widget
         */
        Widget(QWidget* parent, DimensionSelectionAction* dimensionSelectionAction, const Widget::State& state);
    };

    /**
     * Get widget representation of the dimension selection action
     * @param parent Pointer to parent widget
     * @param state Widget state
     */
    QWidget* getWidget(QWidget* parent, const hdps::gui::WidgetActionWidget::State& state = hdps::gui::WidgetActionWidget::State::Standard) override {
        return new Widget(parent, this, state);
    };

public:

    /**
     * Constructor
     * @param parent Pointer to parent object
     */
    DimensionSelectionAction(QObject* parent);

    /** Destructor */
    ~DimensionSelectionAction();

    /**
     * Set dimensions
     * @param numDimensions Number of dimensions
     * @param names Dimension names
     */
    void setDimensions(const std::uint32_t numDimensions, const std::vector<QString>& names);

    /** Get list of enabled dimensions */
    std::vector<bool> getEnabledDimensions() const;

    /**
     * Sets data changed
     * @param points Points dataset
     */
    void dataChanged(Points& points);

    /** Get selection proxy model */
    hdps::DimensionSelectionProxyModel* getProxyModel();

public:

    /**
     * Set name filter
     * @param nameFilter Name filter
     */
    void setNameFilter(const QString& nameFilter);

    /**
     * Set show only selected dimensions
     * @param showOnlySelectedDimensions Show only selected dimensions
     */
    void setShowOnlySelectedDimensions(const bool& showOnlySelectedDimensions);

    /**
     * Set apply exclusion list
     * @param applyExclusionList Apply exclusion list
     */
    void setApplyExclusionList(const bool& applyExclusionList);

    /**
     * Set ignore zero values
     * @param ignoreZeroValues Ignore zero values
     */
    void setIgnoreZeroValues(const bool& ignoreZeroValues);

    /**
     * Load selection from file
     * @param fileName Selection file name
     */
    void loadSelectionFromFile(const QString& fileName);

    /**
     * Load exclusion from file
     * @param fileName Exclusion file name
     */
    void loadExclusionFromFile(const QString& fileName);

    /**
     * Save selection from file
     * @param fileName Selection file name
     */
    void saveSelectionToFile(const QString& fileName);

protected:
    
    /** Compute dimension statistics */
    void computeStatistics();

    /** Update the slider */
    void updateSlider();

    /** Update the dimension selection summary */
    void updateSummary();

    /** Select dimensions based on visibility */
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
    Points*                                             _pointData;                             /** Pointer to points set */
    hdps::DimensionSelectionHolder                      _selectionHolder;                       /** Selection holder */
    std::unique_ptr<hdps::DimensionSelectionItemModel>  _selectionItemModel;                    /** Selection item model */
    std::unique_ptr<hdps::DimensionSelectionProxyModel> _selectionProxyModel;                   /** Selection proxy model for filtering etc. */
    hdps::gui::StringAction                             _nameFilterAction;                      /** Name filter action */
    hdps::gui::ToggleAction                             _showOnlySelectedDimensionsAction;      /** Show only selected dimensions action */
    hdps::gui::ToggleAction                             _applyExclusionListAction;              /** Apply exclusion list action */
    hdps::gui::ToggleAction                             _ignoreZeroValuesAction;                /** Ignore zero values for statistics action */
    hdps::gui::IntegralAction                           _selectionThresholdAction;              /** Selection threshold action */
    hdps::gui::StringAction                             _summaryAction;                         /** Summary action */
    hdps::gui::TriggerAction                            _computeStatisticsAction;               /** Compute statistics action */
    hdps::gui::TriggerAction                            _selectVisibleAction;                   /** Select visible dimensions action */
    hdps::gui::TriggerAction                            _selectNonVisibleAction;                /** Select non visible dimensions action */
    hdps::gui::TriggerAction                            _loadSelectionAction;                   /** Load selection action */
    hdps::gui::TriggerAction                            _saveSelectionAction;                   /** Save selection action */
    hdps::gui::TriggerAction                            _loadExclusionAction;                   /** Load exclusion action */
    QMetaObject::Connection                             _summaryUpdateAwakeConnection;          /** Update summary view when idle */

    friend class Widget;
};