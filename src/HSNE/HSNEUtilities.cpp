#include "HSNEUtilities.h"

#include "HSNEHierarchy.h"

#include <CoreInterface.h>
#include <Dataset.h>
#include <PointData/PointData.h>

#include <cstdint>
#include <vector>

// Publish landmark weight (sum of probabilities that a data point is represented by a landmark)
void publishLandmarkWeightsData(class HsneHierarchy* hsneHierarchy, unsigned int scaleLevel, const mv::Dataset<Points>& embeddingDataset, const std::vector<uint32_t>* landmarkIDs) {
    if (!hsneHierarchy)
        return;

    if (!hsneHierarchy->getPublishLandmarkWeights())
        return;

    const unsigned int numLandmarks   = embeddingDataset->getNumPoints();

    Hsne::scale_type& scale           = hsneHierarchy->getScale(scaleLevel);
    const auto& landmarkWeights       = scale._landmark_weight;

    const QString weightPropertyName  = "hsne-embedding-parent";
    const QString weightPropertyValue = embeddingDataset.getDatasetId();

    if (landmarkWeights.size() != numLandmarks && !landmarkIDs)
        return;

    mv::Dataset<Points> weightDataset;

    for (const auto& childrenHierarchyItem : embeddingDataset->getDataHierarchyItem().getChildren()) {
        const auto childData = childrenHierarchyItem->getDataset();
        if (childData->hasProperty(weightPropertyName) && childData->getProperty(weightPropertyName).toString() == weightPropertyValue) {
            weightDataset = childData;
            break;
        }
    }

    if (!weightDataset.isValid()) {
        weightDataset = mv::Dataset<Points>(mv::data().createDerivedDataset("Landmark weights", embeddingDataset, embeddingDataset));
        weightDataset->setProperty(weightPropertyName, weightPropertyValue);
    }

    if (landmarkWeights.size() == numLandmarks) {
        weightDataset->setData(landmarkWeights, 1);
    }
    else {
        std::vector<float> landmarkWeightsSubset(numLandmarks);

#pragma omp parallel for
        for (std::int64_t i = 0; i < numLandmarks; i++) {
            landmarkWeightsSubset[i] = landmarkWeights[landmarkIDs->at(i)];
        }

        weightDataset->setData(std::move(landmarkWeightsSubset), 1);
    }

    mv::events().notifyDatasetDataChanged(weightDataset);

}