#pragma once

#include "actions/Actions.h"
#include "event/EventListener.h"

#include "TsneAnalysis.h"

class QMenu;
class HsneAnalysisPlugin;
class HsneHierarchy;
class TsneSettingsAction;

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
    HsneScaleAction(QObject* parent, TsneSettingsAction* tsneSettingsAction, hdps::CoreInterface* core, HsneHierarchy* hsneHierarchy, const QString& inputDataSetName, const QString& inputEmbeddingName);

    QMenu* getContextMenu();

    TsneSettingsAction& getTsneSettingsAction() { return *_tsneSettingsAction; }
    hdps::gui::TriggerAction& getRefineAction() { return _refineAction; }

protected:
    void refine();

protected:
    TsneSettingsAction*         _tsneSettingsAction;            /** Pointer to TSNE settings action from the HSNE analysis */
    hdps::CoreInterface*        _core;                          /** Pointer to the core */
    TsneAnalysis                _tsne;                          /** TSNE analysis */
    HsneHierarchy*              _hsneHierarchy;                 /** Pointer to HSNE hierarchy */
    QString                     _inputDatasetName;              /** Name of the input dataset */
    QString                     _inputEmbeddingName;            /** Name of the input embedding dataset */
    QString                     _refineEmbeddingName;           /** Name of the output embedding dataset */
    hdps::gui::TriggerAction    _refineAction;                  /** Refine action */

    friend class Widget;
};