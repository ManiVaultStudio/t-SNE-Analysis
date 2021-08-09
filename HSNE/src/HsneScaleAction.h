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

class HsneScaleAction : public hdps::gui::WidgetActionGroup, public hdps::EventListener
{
protected:

    class Widget : public hdps::gui::WidgetActionGroup::GroupWidget {
    public:
        Widget(QWidget* parent, HsneScaleAction* HsneScaleAction, const Widget::State& state);
    };

    QWidget* getWidget(QWidget* parent, const Widget::State& state = Widget::State::Standard) override {
        return new Widget(parent, this, state);
    };

public:
    HsneScaleAction(QObject* parent, TsneSettingsAction& tsneSettingsAction, HsneHierarchy& hsneHierarchy, hdps::DataHierarchyItem* inputDataHierarchyItem, hdps::DataHierarchyItem* embeddingDataHierarchyItem);

    QMenu* getContextMenu();

    TsneSettingsAction& getTsneSettingsAction() { return _tsneSettingsAction; }
    hdps::gui::TriggerAction& getRefineAction() { return _refineAction; }

protected:
    void refine();

protected:
    TsneSettingsAction&         _tsneSettingsAction;            /** Reference to TSNE settings action from the HSNE analysis */
    TsneAnalysis                _tsne;                          /** TSNE analysis */
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