#pragma once

#include <vector>

#include <Dataset.h>
#include <PointData/PointData.h>

class HsneHierarchy;

void publishLandmarkWeightsData(class HsneHierarchy* hsneHierarchy, unsigned int scaleLevel, const mv::Dataset<Points>& embeddingDataset, const std::vector<uint32_t>* landmarkIDs = nullptr);
