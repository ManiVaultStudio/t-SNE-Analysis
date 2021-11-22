#pragma once

#include "actions/Actions.h"
#include "event/EventListener.h"
//#include "util/SmartDataset.h"

#include "TsneAnalysis.h"
#include "PointData.h"

using namespace hdps;
using namespace hdps::gui;
using namespace hdps::util;

class QMenu;

class HsneAnalysisPlugin;
class HsneHierarchy;
class TsneSettingsAction;

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
class HsneScaleAction : public GroupAction, public EventListener
{
public:

    /**
     * Constructor
     * @param parent Pointer to parent object
     * @param tsneSettingsAction Reference to TSNE settings action
     * @param hsneHierarchy Reference to HSNE hierarchy
     * @param inputDataset Smart pointer to input dataset
     * @param embeddingDataset Smart pointer to embedding dataset
     */
    HsneScaleAction(QObject* parent, TsneSettingsAction& tsneSettingsAction, HsneHierarchy& hsneHierarchy, Dataset<Points>& inputDataset, Dataset<Points>& embeddingDataset);

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
    TsneSettingsAction&     _tsneSettingsAction;    /** Reference to TSNE settings action from the HSNE analysis */
    TsneAnalysis            _tsneAnalysis;          /** TSNE analysis */
    HsneHierarchy&          _hsneHierarchy;         /** Reference to HSNE hierarchy */
    Dataset<Points>         _input;                 /** Input dataset reference */
    Dataset<Points>         _embedding;             /** Embedding dataset reference */
    Dataset<Points>         _refineEmbedding;       /** Refine embedding dataset reference */
    TriggerAction           _refineAction;          /** Refine action */

protected:
    static hdps::CoreInterface* core;      /** Pointer to the core */

    friend class HsneAnalysisPlugin;
    friend class Widget;
};
