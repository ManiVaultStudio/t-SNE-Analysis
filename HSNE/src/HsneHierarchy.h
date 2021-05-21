#pragma once

#include "CoreInterface.h"
#include "TsneAnalysis.h"
#include "hdi/dimensionality_reduction/hierarchical_sne.h"
#include "hdi/utils/graph_algorithms.h"

#include <QString>
#include <QDebug>

#include <vector>
#include <memory>

using HsneMatrix = std::vector<hdi::data::MapMemEff<uint32_t, float>>;
using Hsne = hdi::dr::HierarchicalSNE<float, HsneMatrix>;

class Points;
class HsneParameters;

class HsneHierarchy : public QObject
{
    Q_OBJECT
public:
    /**
     * Initialize the HSNE hierarchy with a data-level scale.
     *
     * @param  data        The high-dimensional data
     * @param  parameters  Parameters with which to run the HSNE algorithm
     */
    void initialize(hdps::CoreInterface* core, const Points& inputData, const std::vector<bool>& enabledDimensions, const HsneParameters& parameters);
    HsneMatrix getTransitionMatrixAtScale(int scale) { return _hsne->scale(scale)._transition_matrix; }

    /**
     * Returns a map of landmark indices and influences on the previous scale in the hierarchy,
     * that are influenced by landmarks specified by their index in the current scale.
     */
    void getInfluencedLandmarksInPreviousScale(int currentScale, std::vector<unsigned int> indices, std::map<uint32_t, float>& neighbors)
    {
        _hsne->getInfluencedLandmarksInPreviousScale(currentScale, indices, neighbors);
    }

    /**
     * Extract a subgraph with the selected indexes and the vertices connected to them by an edge with a weight higher then thresh
     * 
     */
    void getTransitionMatrixForSelection(int currentScale, HsneMatrix& transitionMatrix, std::vector<uint32_t>& landmarkIdxs)
    {
        // Get full transition matrix of the previous scale
        HsneMatrix& fullTransitionMatrix = _hsne->scale(currentScale-1)._transition_matrix;

        // Extract the selected subportion of the transition matrix
        std::vector<unsigned int> dummy;
        hdi::utils::extractSubGraph(fullTransitionMatrix, landmarkIdxs, transitionMatrix, dummy, 1);
    }

    int getNumScales() { return _numScales; }
    QString getInputDataName() { return _inputDataName; }
    int getNumPoints() { return _numPoints; }
    int getNumDimensions() { return _numDimensions; }

public slots:
    void newScale();

private:
    hdps::CoreInterface* _core;

    std::unique_ptr<Hsne> _hsne;

    QString _inputDataName;
    QString _embeddingName;

    int _numScales = 1;

    unsigned int _numPoints;
    unsigned int _numDimensions;
};
