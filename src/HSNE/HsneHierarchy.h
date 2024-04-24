#pragma once

#include "hdi/dimensionality_reduction/hierarchical_sne.h"
#include "hdi/utils/cout_log.h"
#include "hdi/utils/graph_algorithms.h"

#include "PointData/PointData.h"

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <QObject>

using HsneMatrix = std::vector<hdi::data::MapMemEff<std::uint32_t, float>>;
using Hsne = hdi::dr::HierarchicalSNE<float, HsneMatrix>;

class HsneParameters;
class KnnParameters;
class HsneHierarchy;

namespace mv {
    class Task;

    namespace utils {
        class CoutLog;
    }
}

using LandmarkMap = std::vector<std::vector<unsigned int>>;
using Path = std::filesystem::path;

/**
 * InfluenceHierarchy
 *
 * Container class for the mapping of the HSNE scales to the data level
 *
 * @author Julian Thijssen
 */
class InfluenceHierarchy : public QObject
{
    Q_OBJECT

public:
    void initialize(HsneHierarchy& hierarchy);

    std::vector<LandmarkMap>& getMap() { return _influenceMap; }
    const std::vector<LandmarkMap>& getMap() const { return _influenceMap; }

private:
    std::vector<LandmarkMap> _influenceMap;
};

/**
 * HsneHierarchy
 *
 * Wrapper for the HDI HSNE hierarchy
 *
 * @author Julian Thijssen
 */
class HsneHierarchy : public QObject
{
    Q_OBJECT

public slots:
    /**
     * Initialize the HSNE hierarchy with a data-level scale. First call setDataAndParameters() and initParentTask()
     */
    void initialize();

signals:
    void finished();

public:
    void setDataAndParameters(const mv::Dataset<Points>& inputData, const mv::Dataset<Points>& outputData, const HsneParameters& parameters, const KnnParameters& knnParameters, std::vector<bool>&& enabledDimensions);

    // Call before moving this object to another thread
    void initParentTask();

    HsneMatrix getTransitionMatrixAtScale(int scale) { return _hsne->scale(scale)._transition_matrix; }

    void printScaleInfo() const;

    bool isInitialized() const { return _isInit; }

    Hsne& getHsne() { return *_hsne.get(); }
    const Hsne& getHsne() const { return *_hsne.get(); }

    Hsne::scale_type& getScale(int scaleId) { return _hsne->scale(scaleId); }
    const Hsne::scale_type& getScale(int scaleId) const { return _hsne->scale(scaleId); }

    InfluenceHierarchy& getInfluenceHierarchy() { return _influenceHierarchy; }
    const InfluenceHierarchy& getInfluenceHierarchy() const { return _influenceHierarchy; }

    /**
     * Returns a map of landmark indices and influences on the previous scale in the hierarchy,
     * that are influenced by landmarks specified by their index in the current scale.
     */
    void getInfluencedLandmarksInPreviousScale(int currentScale, std::vector<std::uint64_t> indices, std::map<std::uint64_t, float>& neighbors)
    {
        _hsne->getInfluencedLandmarksInPreviousScale(currentScale, indices, neighbors);
    }

    void getInfluenceOnDataPoint(std::uint64_t dataPointId, std::vector<std::unordered_map<std::uint64_t, float>>& influence, float thresh = 0, bool normalized = true)
    {
        _hsne->getInfluenceOnDataPoint(dataPointId, influence, thresh, normalized);
    }

    /**
     * Extract a subgraph with the selected indexes and the vertices connected to them by an edge with a weight higher then thresh
     * 
     */
    void getTransitionMatrixForSelection(int currentScale, HsneMatrix& transitionMatrix, std::vector<uint32_t>& landmarkIdxs)
    {
        assert(currentScale > 0);

        // Get full transition matrix of the previous scale
        HsneMatrix& fullTransitionMatrix = _hsne->scale(currentScale-1)._transition_matrix;

        // Extract the selected subportion of the transition matrix
        std::vector<std::uint32_t> dummy;
        hdi::utils::extractSubGraph(fullTransitionMatrix, landmarkIdxs, transitionMatrix, dummy, 1);
    }

    int getNumScales() const { return _numScales; }
    int getTopScale() const { return _numScales - 1; }
    std::string getInputDataName() const { return _inputDataName; }
    int getNumPoints() const { return _numPoints; }
    int getNumDimensions() const { return _numDimensions; }

    /** Save HSNE hierarchy from this class to disk */
    void saveCacheHsne(const Hsne::Parameters& internalParams) const;

    /** Load HSNE hierarchy from disk */
    bool loadCache(const Hsne::Parameters& internalParams, hdi::utils::CoutLog& log);

protected:
    /** Save HsneHierarchy to disk */
    void saveCacheHsneHierarchy(std::string fileName) const;
    /** Save InfluenceHierarchy to disk */
    void saveCacheHsneInfluenceHierarchy(std::string fileName, const std::vector<LandmarkMap>& influenceHierarchy) const;
    /** Save HSNE parameters to disk */
    void saveCacheParameters(std::string fileName, const Hsne::Parameters& internalParams) const;

    /** Load HsneHierarchy from disk */
    bool loadCacheHsneHierarchy(std::string fileName, hdi::utils::CoutLog& _log);
    /** Load InfluenceHierarchy from disk */
    bool loadCacheHsneInfluenceHierarchy(std::string fileName, std::vector<LandmarkMap>& influenceHierarchy);
    /** Check whether HSNE parameters of the cached values on disk correspond with the current settings */
    bool checkCacheParameters(const std::string fileName, const Hsne::Parameters& params) const;

    void setIsInitialized(bool init) { _isInit = true; }

private:
    std::unique_ptr<Hsne>   _hsne;
    InfluenceHierarchy      _influenceHierarchy;

    std::vector<bool>       _enabledDimensions;
    mv::Dataset<Points>     _inputData;
    mv::Dataset<Points>     _outputData;
    std::string             _inputDataName;
    mv::Task*               _parentTask = nullptr;

    int                     _numScales = 1;
    unsigned int            _numPoints = 0;
    unsigned int            _numDimensions = 0;
    Hsne::Parameters        _params;
    bool                    _isInit = false;

    Path                    _cachePath;                            /** Path for saving and loading cache */
    Path                    _cachePathFileName;                    /** cachePath() + data name */
    bool                    _saveHierarchyToDisk = false;

    friend class HsneAnalysisPlugin;
};
