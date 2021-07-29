#pragma once

#include "CoreInterface.h"
#include "TsneAnalysis.h"
#include "hdi/dimensionality_reduction/hierarchical_sne.h"
#include "hdi/utils/graph_algorithms.h"

#include <QString>
#include <QDebug>

#include <vector>
#include <unordered_map>
#include <memory>

using HsneMatrix = std::vector<hdi::data::MapMemEff<uint32_t, float>>;
using Hsne = hdi::dr::HierarchicalSNE<float, HsneMatrix>;

class Points;
class HsneParameters;

class HsneHierarchy;

using LandmarkMap = std::vector<std::vector<unsigned int>>;

class InfluenceHierarchy
{
public:
    void initialize(HsneHierarchy& hierarchy);

    std::vector<LandmarkMap>& getMap() { return _influenceMap; }

private:
    std::vector<LandmarkMap> _influenceMap;
};

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

    void printScaleInfo()
    {
        std::cout << "Landmark to Orig size: " << _hsne->scale(getNumScales() - 1)._landmark_to_original_data_idx.size() << std::endl;
        std::cout << "Landmark to Prev size: " << _hsne->scale(getNumScales() - 1)._landmark_to_previous_scale_idx.size() << std::endl;
        std::cout << "Prev to Landmark size: " << _hsne->scale(getNumScales() - 1)._previous_scale_to_landmark_idx.size() << std::endl;
        std::cout << "AoI size: " << _hsne->scale(getNumScales() - 1)._area_of_influence.size() << std::endl;
    }

    Hsne::scale_type& getScale(int scaleId)
    {
        return _hsne->scale(scaleId);
    }

    InfluenceHierarchy& getInfluenceHierarchy()
    {
        return _influenceHierarchy;
    }

    /**
     * Returns a map of landmark indices and influences on the previous scale in the hierarchy,
     * that are influenced by landmarks specified by their index in the current scale.
     */
    void getInfluencedLandmarksInPreviousScale(int currentScale, std::vector<unsigned int> indices, std::map<uint32_t, float>& neighbors)
    {
        _hsne->getInfluencedLandmarksInPreviousScale(currentScale, indices, neighbors);
    }

    void getInfluenceOnDataPoint(unsigned int dataPointId, std::vector<std::unordered_map<unsigned int, float>>& influence, float thresh = 0, bool normalized = true)
    {
        _hsne->getInfluenceOnDataPoint(dataPointId, influence, thresh, normalized);
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
    int getTopScale() { return _numScales - 1; }
    QString getInputDataName() { return _inputDataName; }
    int getNumPoints() { return _numPoints; }
    int getNumDimensions() { return _numDimensions; }

public slots:
    void newScale();

private:
    hdps::CoreInterface* _core;

    std::unique_ptr<Hsne> _hsne;

    InfluenceHierarchy _influenceHierarchy;

    QString _inputDataName;
    QString _embeddingName;

    int _numScales = 1;

    unsigned int _numPoints;
    unsigned int _numDimensions;
};
