#pragma once

#include "actions/Actions.h"
#include "event/EventListener.h"
//#include "util/SmartDataset.h"

#include "TsneAnalysis.h"

#include "PointData/PointData.h"
#include "PointData/DimensionsPickerAction.h"

using namespace mv;
using namespace mv::gui;
using namespace mv::util;

class QMenu;

class HsneAnalysisPlugin;
class HsneHierarchy;
class TsneSettingsAction;

namespace mv {
    class CoreInterface;
}

/**
 * HSNE scale action class
 *
 * Action class for HSNE scale
 *
 * @author Thomas Kroes
 */
class HsneScaleAction : public GroupAction
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
    HsneScaleAction(QObject* parent, TsneSettingsAction& tsneSettingsAction, HsneHierarchy& hsneHierarchy, Dataset<Points> inputDataset, Dataset<Points> embeddingDataset);

    /**
     * Get the context menu for the action
     * @param parent Parent widget
     * @return Context menu
     */
    QMenu* getContextMenu(QWidget* parent = nullptr) override;

private:
    /** Refine the landmarks based on the current selection */
    void refine();

public: // Action getters

    TsneSettingsAction& getTsneSettingsAction() { return _tsneSettingsAction; }
    TriggerAction& getRefineAction() { return _refineAction; }

public: // Setters
    void setScale(unsigned int scale)
    {
        _currentScaleLevel = scale;
    }

    void setDrillIndices(const std::vector<uint32_t>& drillIndices)
    {
        _drillIndices = drillIndices;
        _isTopScale = false;
    }

public: // Serialization

    /**
     * Load plugin from variant map
     * @param Variant map representation of the plugin
     */
    void fromVariantMap(const QVariantMap& variantMap) override;

    /**
     * Save plugin to variant map
     * @return Variant map representation of the plugin
     */
    QVariantMap toVariantMap() const override;

private:
    TsneSettingsAction&     _tsneSettingsAction;    /** Reference to TSNE settings action from the HSNE analysis */
    TsneAnalysis            _tsneAnalysis;          /** TSNE analysis */
    HsneHierarchy&          _hsneHierarchy;         /** Reference to HSNE hierarchy */
    Dataset<Points>         _input;                 /** Input dataset reference */
    Dataset<Points>         _embedding;             /** Embedding dataset reference */
    Dataset<Points>         _refineEmbedding;       /** Refine embedding dataset reference */

    TriggerAction           _refineAction;          /** Refine action */
    DatasetPickerAction     _datasetPickerAction;   /** Dataset picker action */
    TriggerAction           _reloadDatasetsAction;  /** Reload action */
    TriggerAction           _setSelectionAction;    /** Set selection action */

    EventListener           _eventListener;         /** Listen to HDPS events */
    mv::ForegroundTask      _initializationTask;    /** Task for reporting computation preparation progress */

private:
    std::vector<uint32_t>   _drillIndices;          /** Vector relating local indices to scale relative indices */
    bool                    _isTopScale;            /** Whether current scale is the top scale */
    unsigned int            _currentScaleLevel;     /** The scale the current embedding is a part of */
};
