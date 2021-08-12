#pragma once

#include "actions/Actions.h"
#include "event/EventListener.h"

#include "TsneAnalysis.h"

class QMenu;
class HsneAnalysisPlugin;
class HsneHierarchy;
class TsneSettingsAction;
class DataHierarchyItem;

namespace hdps {
    class CoreInterface;
}

/**
 * HSNE scale action class
 *
 * Action class for HSNE scale
 *
 * @author Thomas Kroes
 */
class HsneScaleAction : public hdps::gui::WidgetActionGroup, public hdps::EventListener
{
protected:

    /** Widget class for HSNE scale action */
    class Widget : public hdps::gui::WidgetActionGroup::FormWidget {
    public:

        /**
         * Constructor
         * @param parent Pointer to parent widget
         * @param hsneScaleAction Pointer to HSNE scale action
         * @param state State of the widget
         */
        Widget(QWidget* parent, HsneScaleAction* hsneScaleAction, const hdps::gui::WidgetActionWidget::State& state);
    };

    /**
     * Get widget representation of the HSNE scale action
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
     * @param tsneSettingsAction Reference to TSNE settings action
     * @param hsneHierarchy Reference to HSNE hierarchy
     * @param inputDataHierarchyItem Pointer to input data hierarchy item
     * @param embeddingDataHierarchyItem Pointer to embedding data hierarchy item
     */
    HsneScaleAction(QObject* parent, TsneSettingsAction& tsneSettingsAction, HsneHierarchy& hsneHierarchy, hdps::DataHierarchyItem* inputDataHierarchyItem, hdps::DataHierarchyItem* embeddingDataHierarchyItem);

    /**
     * Get the context menu for the action
     * @param parent Parent widget
     * @return Context menu
     */
    QMenu* getContextMenu(QWidget* parent = nullptr) override;

protected:

    /** Refine the landmarks based on the current selection */
    void refine();

public: // Action getters

    TsneSettingsAction& getTsneSettingsAction() { return _tsneSettingsAction; }
    hdps::gui::TriggerAction& getRefineAction() { return _refineAction; }

protected:
    TsneSettingsAction&         _tsneSettingsAction;            /** Reference to TSNE settings action from the HSNE analysis */
    TsneAnalysis                _tsneAnalysis;                          /** TSNE analysis */
    HsneHierarchy&              _hsneHierarchy;                 /** Reference to HSNE hierarchy */
    hdps::DataHierarchyItem*    _inputDataHierarchyItem;        /** Input data hierarchy item */
    hdps::DataHierarchyItem*    _embeddingDataHierarchyItem;    /** Embedding data hierarchy item */
    QString                     _refineEmbeddingName;           /** Name of the output embedding dataset */
    hdps::gui::TriggerAction    _refineAction;                  /** Refine action */

protected:
    static hdps::CoreInterface* core;      /** Pointer to the core */

    friend class HsneAnalysisPlugin;
    friend class Widget;
};
