#pragma once


#include "actions/GroupAction.h"
#include "actions/IntegralAction.h"
#include "actions/TriggerAction.h"

#include "event/EventListener.h"

#include "TsneAnalysis.h"
#include "TsneComputationAction.h"

#include "PointData/PointData.h"

#include <memory>

using namespace mv;
using namespace mv::gui;
using namespace mv::util;

class QMenu;

class HsneAnalysisPlugin;
class HsneHierarchy;
class TsneParameters;

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
    HsneScaleAction(QObject* parent, TsneParameters& tsneParameters, HsneHierarchy& hsneHierarchy, Dataset<Points> inputDataset, Dataset<Points> embeddingDataset);

    ~HsneScaleAction();

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

    TriggerAction& getRefineAction() { return _refineAction; }
    IntegralAction& getNumIterationsAction() { return _numIterationsAction; };
    IntegralAction& getNumberOfComputatedIterationsAction() { return _numberOfComputatedIterationsAction; };
    TsneComputationAction& getComputationAction() { return _computationAction; }

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
    using Datasets = std::vector<Dataset<Points>>;
    using RefineActions = std::vector<HsneScaleAction*>;

private:
    TsneParameters&         _tsneParameters;        /** Reference to TSNE paremeters from the HSNE analysis */
    TsneAnalysis            _tsneAnalysis;          /** TSNE analysis */
    HsneHierarchy&          _hsneHierarchy;         /** Reference to HSNE hierarchy */
    Dataset<Points>         _input;                 /** Input dataset reference */
    Dataset<Points>         _embedding;             /** Embedding dataset reference */
    Datasets                _refineEmbeddings;      /** Refine embedding dataset references */

private:
    TriggerAction           _refineAction;                          /** Refine action */
    IntegralAction          _numIterationsAction;                   /** Number of iterations action */
    IntegralAction          _numberOfComputatedIterationsAction;    /** Number of computed iterations action */
    IntegralAction          _updateIterationsAction;                /** Number of update iterations (copying embedding to ManiVault core) */
    TsneComputationAction   _computationAction;                     /** Computation action */

    EventListener           _eventListener;         /** Listen to HDPS events */
    mv::ForegroundTask      _initializationTask;    /** Task for reporting computation preparation progress */

    RefineActions           _refinedScaledActions;  /** Scale actions of the refined datasets */

protected:
    std::vector<uint32_t>   _drillIndices;          /** Vector relating local indices to scale relative indices */
    bool                    _isTopScale;            /** Whether current scale is the top scale */
    unsigned int            _currentScaleLevel;     /** The scale the current embedding is a part of */

    friend class HsneScaleAction;
};
