#include "HsneHierarchy.h"

#include "HsneParameters.h"
#include "KnnParameters.h"

#include "DataHierarchyItem.h"
#include "ImageData/Images.h"
#include "PointData/PointData.h"

#include "hdi/utils/cout_log.h"

#include <fstream>
#include <iostream>

#include "nlohmann/json.hpp"

#include <QFileInfo>
#include <QString>

// set suffix strings for cache
constexpr auto _CACHE_SUBFOLDER_ = "hsne-cache";
constexpr auto _HIERARCHY_CACHE_EXTENSION_ = "_hierarchy.hsne";
constexpr auto _INFLUENCE_TOPDOWN_CACHE_EXTENSION_ = "_influence-tp-hierarchy.hsne";
constexpr auto _PARAMETERS_CACHE_EXTENSION_ = "_parameters.hsne";
constexpr auto _PARAMETERS_CACHE_VERSION_ = "1.0";

namespace
{
    Hsne::Parameters setParameters(HsneParameters parameters, KnnParameters knnParameters)
    {
        Hsne::Parameters params;
        params._aknn_algorithm = knnParameters.getKnnAlgorithm();
        params._aknn_metric = knnParameters.getKnnDistanceMetric();
        params._aknn_num_checks = static_cast<uint32_t>(knnParameters.getAnnoyNumChecks());
        params._aknn_num_trees = static_cast<uint32_t>(knnParameters.getAnnoyNumTrees());
        params._aknn_algorithmP1 = static_cast<double>(knnParameters.getHNSWm());
        params._aknn_algorithmP2 = static_cast<double>(knnParameters.getHNSWef());

        params._seed = parameters.getSeed();
        params._num_walks_per_landmark = parameters.getNumWalksForAreaOfInfluence();
        params._monte_carlo_sampling = parameters.useMonteCarloSampling();
        params._mcmcs_num_walks = parameters.getNumWalksForLandmarkSelection();
        params._mcmcs_landmark_thresh = parameters.getNumWalksForLandmarkSelectionThreshold();
        params._mcmcs_walk_length = parameters.getRandomWalkLength();
        params._transition_matrix_prune_thresh = parameters.getMinWalksRequired();
        params._out_of_core_computation = parameters.useOutOfCoreComputation();
        params._num_neighbors = parameters.getNumNearestNeighbors();
        return params;
    }
}

/**
 * Compute for every scale except the bottom scale, which landmark influences which bottom scale point
 */
void InfluenceHierarchy::initialize(HsneHierarchy& hierarchy)
{
    _influenceMap.resize(hierarchy.getNumScales());

    // For every scale except the bottom scale resize the landmark map to the number of landmarks
    for (int scale = 1; scale < hierarchy.getNumScales(); scale++)
    {
        int numLandmarks = hierarchy.getScale(scale).size();

        _influenceMap[scale].resize(numLandmarks);
    }

    auto& bottomScale = hierarchy.getScale(0);

    int numDataPoints = bottomScale.size();

#pragma omp parallel for
    for (int i = 0; i < numDataPoints; i++)
    {
        std::vector<std::unordered_map<unsigned int, float>> influence;

        float thresh = 0.01f;

        hierarchy.getInfluenceOnDataPoint(i, influence, thresh, false);
        ///////
        int redo = 1;
        int tries = 0;
        while (redo)
        {
            redo = 0;
            if (tries++ < 3)
            {
                for (int scale = 1; scale < hierarchy.getNumScales(); scale++)
                {
                    if (influence[scale].size() < 1)
                    {
                        redo = scale;
                    }
                }
            }
            if (redo > 0)
            {
                //if(thresh < 0.0005) std::cout << "Couldn't find landmark for point " << i << " at scale " << redo << " num possible landmarks " << influence[redo].size() << "\nSetting new threshold to " << thresh * 0.1 << std::endl;
                thresh *= 0.1;
                hierarchy.getInfluenceOnDataPoint(i, influence, thresh, false);
            }
        }
        //////

        for (int scale = 1; scale < hierarchy.getNumScales(); scale++)
        {
            float maxInfluence = 0;
            int topInfluencingLandmark = -1;

            for (auto& landmark : influence[scale])
            {
                if (landmark.second >= maxInfluence)
                {
                    maxInfluence = landmark.second;
                    topInfluencingLandmark = landmark.first;
                }
            }

            if (topInfluencingLandmark == -1)
            {
                std::cerr << "Failed to find landmark for point " << i << " at scale " << scale << " num possible landmarks " << influence[scale].size() << std::endl;
                continue;
            }

            #pragma omp critical
            {
                _influenceMap[scale][topInfluencingLandmark].push_back(i);
            }
        }
    }
}

void HsneHierarchy::printScaleInfo() const
{
    std::cout << "Landmark to Orig size: " << _hsne->scale(getNumScales() - 1)._landmark_to_original_data_idx.size() << std::endl;
    std::cout << "Landmark to Prev size: " << _hsne->scale(getNumScales() - 1)._landmark_to_previous_scale_idx.size() << std::endl;
    std::cout << "Prev to Landmark size: " << _hsne->scale(getNumScales() - 1)._previous_scale_to_landmark_idx.size() << std::endl;
    std::cout << "AoI size: " << _hsne->scale(getNumScales() - 1)._area_of_influence.size() << std::endl;
}

void HsneHierarchy::setDataAndParameters(const Points& inputData, const std::vector<bool>& enabledDimensions, const HsneParameters& parameters, const KnnParameters& knnParameters)
{
    // Convert our own HSNE parameters to the HDI parameters
    _params = setParameters(parameters, knnParameters);

    // Extract the enabled dimensions from the data
    unsigned int numEnabledDimensions = count_if(enabledDimensions.begin(), enabledDimensions.end(), [](bool b) { return b; });

    // Get data and hierarchy info
    _numScales = parameters.getNumScales();
    _numPoints = inputData.getNumPoints();
    _numDimensions = numEnabledDimensions;

    // Check for source data file path
    std::string inputLoadPath = std::string();
    {
        mv::Dataset<Images> inputdataImage = nullptr;
        for (auto childHierarchyItem : inputData.getDataHierarchyItem().getChildren()) {

            if (childHierarchyItem->getDataType() == ImageType) {
                inputdataImage = childHierarchyItem->getDataset();
                break;
            }
        }

        if (inputdataImage.isValid())
        {
            auto imageFilePath = inputdataImage->getImageFilePaths();
            if(!imageFilePath.isEmpty())
                inputLoadPath = QFileInfo(imageFilePath.first()).dir().absolutePath().toStdString();
        }
    }

    // Set cache paths
    if (inputLoadPath == std::string())
        _cachePath = std::filesystem::current_path() / _CACHE_SUBFOLDER_;
    else
        _cachePath = std::filesystem::path(inputLoadPath) / _CACHE_SUBFOLDER_;

    _inputDataName = inputData.text().toStdString();
    _cachePathFileName = _cachePath / _inputDataName;

    _hsne = std::make_unique<Hsne>();
}

void HsneHierarchy::initialize(const Points& inputData, const std::vector<bool>& enabledDimensions, const HsneParameters& parameters, const KnnParameters& knnParameters)
{
    setDataAndParameters(inputData, enabledDimensions, parameters, knnParameters);

    hdi::utils::CoutLog log;

    // Check of hsne data can be loaded from cache on disk, otherwise compute hsne hierarchy
    bool hsneLoadedFromCache = loadCache(_params, log);
    if (hsneLoadedFromCache == false) {
        std::cout << "Initializing HSNE hierarchy " << std::endl;

        // Set up a logger
        _hsne->setLogger(&log);

        // Set the dimensionality of the data in the HSNE object
        _hsne->setDimensionality(_numDimensions);

        // Load data and enabled dimensions
        std::vector<float> data;
        std::vector<unsigned int> dimensionIndices;
        data.resize((inputData.isFull() ? inputData.getNumPoints() : inputData.indices.size()) * _numDimensions);
        for (int i = 0; i < inputData.getNumDimensions(); i++)
            if (enabledDimensions[i]) dimensionIndices.push_back(i);

        inputData.populateDataForDimensions<std::vector<float>, std::vector<unsigned int>>(data, dimensionIndices);

        // Initialize HSNE with the input data and the given parameters
        _hsne->initialize((Hsne::scalar_type*)data.data(), _numPoints, _params);

        // Add a number of scales as indicated by the user
        for (int s = 0; s < _numScales - 1; ++s) {
            _hsne->addScale();
        }

        _influenceHierarchy.initialize(*this);

        // Write HSNE hierarchy to disk
        if(parameters.getSaveHierarchyToDisk())
            saveCacheHsne(_params); 
    }

    _isInit = true;
}


void HsneHierarchy::saveCacheHsne(const Hsne::Parameters& internalParams) const {
    if (!_hsne) return; // only save if initialize() has been called

    if (!std::filesystem::exists(_cachePath))
        std::filesystem::create_directory(_cachePath);

    std::cout << "HsneHierarchy::saveCacheHsne(): save cache to " + _cachePathFileName.string() << std::endl;

    saveCacheHsneHierarchy(_cachePathFileName.string() + _HIERARCHY_CACHE_EXTENSION_);
    saveCacheHsneInfluenceHierarchy(_cachePathFileName.string() + _INFLUENCE_TOPDOWN_CACHE_EXTENSION_, _influenceHierarchy.getMap());
    saveCacheParameters(_cachePathFileName.string() + _PARAMETERS_CACHE_EXTENSION_, internalParams);
}

void HsneHierarchy::saveCacheHsneHierarchy(std::string fileName) const {
    std::cout << "Writing " + fileName << std::endl;

    std::ofstream saveFile(fileName, std::ios::out | std::ios::binary);

    if (!saveFile.is_open())
    {
        std::cerr << "Caching failed. File could not be opened. " << std::endl;
        return;
    }

    hdi::dr::IO::saveHSNE(*_hsne, saveFile, _hsne->logger());

    saveFile.close();

}

void HsneHierarchy::saveCacheHsneInfluenceHierarchy(std::string fileName, const std::vector<LandmarkMap>& influenceHierarchy) const {
    std::cout << "Writing " + fileName << std::endl;

    std::ofstream saveFile(fileName, std::ios::out | std::ios::binary);

    if (!saveFile.is_open())
    {
        std::cerr << "Caching failed. File could not be opened. " << std::endl;
        return;
    }

    size_t iSize = influenceHierarchy.size();

    saveFile.write((const char*)&iSize, sizeof(decltype(iSize)));
    for (size_t i = 0; i < iSize; i++)
    {
        size_t jSize = influenceHierarchy[i].size();
        saveFile.write((const char*)&jSize, sizeof(decltype(jSize)));
        for (size_t j = 0; j < jSize; j++)
        {
            size_t kSize = influenceHierarchy[i][j].size();
            saveFile.write((const char*)&kSize, sizeof(decltype(kSize)));
            if (kSize > 0)
            {
                saveFile.write((const char*)&influenceHierarchy[i][j].front(), kSize * sizeof(uint32_t));
            }
        }
    }

    saveFile.close();
}


void HsneHierarchy::saveCacheParameters(std::string fileName, const Hsne::Parameters& internalParams) const {
    std::cout << "Writing " + fileName << std::endl;

    std::ofstream saveFile(fileName, std::ios::out | std::ios::trunc);

    if (!saveFile.is_open())
    {
        std::cerr << "Caching failed. File could not be opened. " << std::endl;
        return;
    }

    // store parameters in json file
    nlohmann::json parameters;
    parameters["## VERSION ##"] = _PARAMETERS_CACHE_VERSION_;

    parameters["Input data name"] = _inputDataName;
    parameters["Number of points"] = _numPoints;
    parameters["Number of dimensions"] = _numDimensions;

    parameters["Number of Scales"] = _numScales;

    parameters["Knn library"] = internalParams._aknn_algorithm;
    parameters["Knn distance metric"] = internalParams._aknn_metric;
    parameters["Knn number of neighbors"] = internalParams._num_neighbors;

    parameters["Nr. Checks in AKNN"] = internalParams._aknn_num_checks;
    parameters["Nr. Trees for AKNN"] = internalParams._aknn_num_trees;

    parameters["Memory preserving computation"] = internalParams._out_of_core_computation;
    parameters["Nr. RW for influence"] = internalParams._num_walks_per_landmark;
    parameters["Nr. RW for Monte Carlo"] = internalParams._mcmcs_num_walks;
    parameters["Random walks threshold"] = internalParams._mcmcs_landmark_thresh;
    parameters["Random walks length"] = internalParams._mcmcs_walk_length;
    parameters["Pruning threshold"] = internalParams._transition_matrix_prune_thresh;
    parameters["Fixed Percentile Landmark Selection"] = internalParams._hard_cut_off;
    parameters["Percentile Landmark Selection"] = internalParams._hard_cut_off_percentage;

    parameters["Seed for random algorithms"] = internalParams._seed;
    parameters["Select landmarks with a MCMCS"] = internalParams._monte_carlo_sampling;

    // Write to file
    saveFile << std::setw(4) << parameters << std::endl;
    saveFile.close();
}


bool HsneHierarchy::loadCache(const Hsne::Parameters& internalParams, hdi::utils::CoutLog& log) {
    std::cout << "HsneHierarchy::loadCache(): attempt to load cache from " + _cachePathFileName.string() << std::endl;

    auto pathParameter = _cachePathFileName.string() + _PARAMETERS_CACHE_EXTENSION_;
    auto pathHierarchy = _cachePathFileName.string() + _HIERARCHY_CACHE_EXTENSION_;
    auto pathInfluenceTD = _cachePathFileName.string() + _INFLUENCE_TOPDOWN_CACHE_EXTENSION_;

    for (const Path& path : { pathHierarchy, pathInfluenceTD, pathParameter })
    {
        if (!(std::filesystem::exists(path)))
        {
            std::cerr << "Loading cache failed: File could not be opened: " + path.string() << std::endl;
            return false;
        }
    }

    if (!checkCacheParameters(pathParameter, internalParams))
    {
        std::cout << "Loading cache failed: Current settings are different from cached parameters." << std::endl;
        return false;
    }

    auto checkChache = [](bool val, std::string path) {
        if (val)
            return true;
        else
        {
            std::cerr << "Loading cache failed: " + path << std::endl;
            return false;
        }
    };

    _isInit = checkChache(loadCacheHsneHierarchy(pathHierarchy, log), pathHierarchy) &&
              checkChache(loadCacheHsneInfluenceHierarchy(pathInfluenceTD, _influenceHierarchy.getMap()), pathInfluenceTD);

    return _isInit;
}

bool HsneHierarchy::loadCacheHsneHierarchy(std::string fileName, hdi::utils::CoutLog& log) {
    std::ifstream loadFile(fileName.c_str(), std::ios::in | std::ios::binary);

    if (!loadFile.is_open()) return false;

    std::cout << "Loading " + fileName << std::endl;
    // TODO: check if hsne matches data

    if (_hsne) {
        _hsne.reset(new Hsne());
    }

    _hsne->setLogger(&log);

    hdi::dr::IO::loadHSNE(*_hsne, loadFile, &log);

    _numScales = static_cast<uint32_t>(_hsne->hierarchy().size());

    return true;

}

bool HsneHierarchy::loadCacheHsneInfluenceHierarchy(std::string fileName, std::vector<LandmarkMap>& influenceHierarchy) {
    if (!_hsne) return false;

    std::ifstream loadFile(fileName.c_str(), std::ios::in | std::ios::binary);

    if (!loadFile.is_open()) return false;

    std::cout << "Loading " + fileName << std::endl;

    // TODO: check if hsne matches data
    size_t iSize = 0;
    loadFile.read((char*)&iSize, sizeof(decltype(iSize)));

    influenceHierarchy.resize(iSize);

    for (size_t i = 0; i < influenceHierarchy.size(); i++)
    {
        size_t jSize = 0;
        loadFile.read((char*)&jSize, sizeof(decltype(jSize)));
        influenceHierarchy[i].resize(jSize);

        for (size_t j = 0; j < influenceHierarchy[i].size(); j++)
        {
            size_t kSize = 0;
            loadFile.read((char*)&kSize, sizeof(decltype(kSize)));

            influenceHierarchy[i][j].resize(kSize);
            if (kSize > 0)
            {
                loadFile.read((char*)&influenceHierarchy[i][j].front(), kSize * sizeof(uint32_t));
            }
        }
    }

    loadFile.close();

    return true;

}

bool HsneHierarchy::checkCacheParameters(const std::string fileName, const Hsne::Parameters& params) const {
    if (!_hsne) return false;

    std::ifstream loadFile(fileName.c_str(), std::ios::in);

    if (!loadFile.is_open()) return false;

    std::cout << "Loading " + fileName << std::endl;

    // read a JSON file
    nlohmann::json parameters;
    loadFile >> parameters;

    if (!parameters.contains("## VERSION ##") || parameters["## VERSION ##"] != _PARAMETERS_CACHE_VERSION_)
    {
        std::cout << "Version of the cache (" + std::string(parameters["## VERSION ##"]) + ") differs from analysis version (" + _PARAMETERS_CACHE_VERSION_ + "). Cannot load cache)" << std::endl;
        return false;
    }

    // if current setting is different from params on disk, don't load from disk
    auto checkParam = [&parameters](std::string paramName, auto localParam) -> bool {
        auto storedParam = parameters[paramName];
        if (storedParam != localParam)
        {
            std::ostringstream localParamSS, storedParamSS;
            localParamSS << localParam;
            storedParamSS << storedParam;
            std::cout << paramName + " (" + localParamSS.str() + ") does not match cache (" + storedParamSS.str() + "). Cannot load cache." << std::endl;
            return false;
        }
        return true;
    };

    if (!checkParam("Input data name", _inputDataName)) return false;
    if (!checkParam("Number of points", _numPoints)) return false;
    if (!checkParam("Number of dimensions", _numDimensions)) return false;

    if (!checkParam("Number of Scales", _numScales)) return false;

    if (!checkParam("Knn library", params._aknn_algorithm)) return false;
    if (!checkParam("Knn distance metric", params._aknn_metric)) return false;
    //if (!checkParam("Knn number of neighbors", params._num_neighbors)) return false;

    //if (!checkParam("Nr. Checks in AKNN", params._aknn_num_checks)) return false;
    //if (!checkParam("Nr. Trees for AKNN", params._aknn_num_trees)) return false;

    //if (!checkParam("Memory preserving computation", params._out_of_core_computation)) return false;
    //if (!checkParam("Nr. RW for influence", params._num_walks_per_landmark)) return false;
    //if (!checkParam("Nr. RW for Monte Carlo", params._mcmcs_num_walks)) return false;
    //if (!checkParam("Random walks threshold", params._mcmcs_landmark_thresh)) return false;
    //if (!checkParam("Random walks length", params._mcmcs_walk_length)) return false;
    //if (!checkParam("Pruning threshold", params._transition_matrix_prune_thresh)) return false;
    //if (!checkParam("Fixed Percentile Landmark Selection", params._hard_cut_off)) return false;
    //if (!checkParam("Percentile Landmark Selection", params._hard_cut_off_percentage)) return false;

    if (!checkParam("Seed for random algorithms", params._seed)) return false;
    if (!checkParam("Select landmarks with a MCMCS", params._monte_carlo_sampling)) return false;

    std::cout << "Parameters of cache correspond to current settings." << std::endl;

    return true;
}