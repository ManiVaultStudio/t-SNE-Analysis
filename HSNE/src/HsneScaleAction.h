#pragma once

#include "actions/Actions.h"
#include "event/EventListener.h"

#include "TsneAnalysis.h"

using namespace hdps;
using namespace hdps::gui;

class QMenu;

class HsneAnalysisPlugin;
class HsneHierarchy;
class TsneSettingsAction;

namespace hdps {
    class CoreInterface;
    class DataHierarchyItem;
}

/**
 * HSNE scale action class
 *
 * Action class for HSNE scale
 *
 * @author Thomas Kroes
 */
class HsneScaleAction : public GroupAction, public EventListener
{
public:

    /**
     * Constructor
     * @param parent Pointer to parent object
     * @param tsneSettingsAction Reference to TSNE settings action
     * @param hsneHierarchy Reference to HSNE hierarchy
     * @param inputDataHierarchyItem Pointer to input data hierarchy item
     * @param embeddingDataHierarchyItem Pointer to embedding data hierarchy item
     */
    HsneScaleAction(QObject* parent, TsneSettingsAction& tsneSettingsAction, HsneHierarchy& hsneHierarchy, DataHierarchyItem* inputDataHierarchyItem, DataHierarchyItem* embeddingDataHierarchyItem);

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
    TriggerAction& getRefineAction() { return _refineAction; }

protected:
    TsneSettingsAction&     _tsneSettingsAction;            /** Reference to TSNE settings action from the HSNE analysis */
    TsneAnalysis            _tsneAnalysis;                  /** TSNE analysis */
    HsneHierarchy&          _hsneHierarchy;                 /** Reference to HSNE hierarchy */
    DataHierarchyItem*      _inputDataHierarchyItem;        /** Input data hierarchy item */
    DataHierarchyItem*      _embeddingDataHierarchyItem;    /** Embedding data hierarchy item */
    QString                 _refineEmbeddingName;           /** Name of the output embedding dataset */
    TriggerAction           _refineAction;                  /** Refine action */

protected:
    static hdps::CoreInterface* core;      /** Pointer to the core */

    friend class HsneAnalysisPlugin;
    friend class Widget;
};
