#pragma once

#include "hdi/dimensionality_reduction/knn_utils.h"

/**
 * KnnParameters
 *
 * Container class for the parameters associated with the knn algorithm
 *
 * @author Alexander Vieth
 */
class KnnParameters
{
public:
    KnnParameters() :
        _knnLibrary(hdi::dr::KNN_FLANN),
        _aknn_metric(hdi::dr::KNN_METRIC_EUCLIDEAN),
        _AnnoyNumChecksAknn(512),
        _AnnoyNumTrees(4),
        _HNSW_M(16),
        _HNSW_ef_construction(200)
    {

    }
    void setKnnAlgorithm(hdi::dr::knn_library knnLibrary) { _knnLibrary = knnLibrary; }
    void setKnnDistanceMetric(hdi::dr::knn_distance_metric knnDistanceMetric) { _aknn_metric = knnDistanceMetric; }
    void setAnnoyNumChecks(int numChecks) { _AnnoyNumChecksAknn = numChecks; }
    void setAnnoyNumTrees(int numTrees) { _AnnoyNumTrees = numTrees; }
    void setHNSWm(int m) { _HNSW_M = m; }
    void setHNSWef(int ef) { _HNSW_ef_construction = ef; }

    hdi::dr::knn_library getKnnAlgorithm() const { return _knnLibrary; }
    hdi::dr::knn_distance_metric getKnnDistanceMetric() const { return _aknn_metric; }
    int getAnnoyNumChecks() const { return _AnnoyNumChecksAknn; }
    int getAnnoyNumTrees() const { return _AnnoyNumTrees; }
    int getHNSWm() const { return _HNSW_M; }
    int getHNSWef() const { return _HNSW_ef_construction; }

private:
    
    hdi::dr::knn_library _knnLibrary;               /** Enum specifying which approximate nearest neighbour library to use for the similarity computation */
    hdi::dr::knn_distance_metric _aknn_metric;      /** Enum specifying which distance to compute knn with */
    
    int _AnnoyNumChecksAknn;                        /** Number of checks used in Annoy, more checks means more precision but slower computation */
    int _AnnoyNumTrees;                             /** Number of trees used in Annoy, more checks means more precision but slower computation */

    int            _HNSW_M;                         /** hnsw: construction time/accuracy trade-off  */
    int            _HNSW_ef_construction;           /** hnsw: maximum number of outgoing connections in the graph  */
};
