#include "HsneSettingsAction.h"

#include "HsneAnalysisPlugin.h"

using namespace mv::gui;

HsneSettingsAction::HsneSettingsAction(HsneAnalysisPlugin* hsneAnalysisPlugin) :
    GroupAction(hsneAnalysisPlugin, "HSNE Settings", true),
    _hsneParameters(),
    _tsneParameters(),
    _knnParameters(),
    _hsneAnalysisPlugin(hsneAnalysisPlugin),
    _generalHsneSettingsAction(*this),
    _hierarchyConstructionSettingsAction(*this),
    _gradientDescentSettingsAction(this, _tsneParameters),
    _knnSettingsAction(this, _knnParameters),
    _topLevelScaleAction(this, hsneAnalysisPlugin->getHierarchy(), hsneAnalysisPlugin->getInputDataset<Points>(), hsneAnalysisPlugin->getOutputDataset<Points>(), &_tsneParameters)
{
    const auto updateReadOnly = [this]() -> void {
        _generalHsneSettingsAction.setReadOnly(isReadOnly());
        _hierarchyConstructionSettingsAction.setReadOnly(isReadOnly());
        _gradientDescentSettingsAction.setReadOnly(isReadOnly());
        _knnSettingsAction.setReadOnly(isReadOnly());
        _topLevelScaleAction.setReadOnly(isReadOnly());
        };

    connect(this, &GroupAction::readOnlyChanged, this, [this, updateReadOnly](const bool& readOnly) {
        updateReadOnly();
    });

    updateReadOnly();
}

void HsneSettingsAction::fromVariantMap(const QVariantMap& variantMap)
{
    GroupAction::fromVariantMap(variantMap);

    variantMapMustContain(variantMap, "HSNE");
    variantMapMustContain(variantMap, "Hierarchy Construction");
    variantMapMustContain(variantMap, "Gradient Descent Settings");
    variantMapMustContain(variantMap, "Knn Settings");
    variantMapMustContain(variantMap, "HSNE Scale");

    _generalHsneSettingsAction.fromVariantMap(variantMap["HSNE"].toMap());
    _hierarchyConstructionSettingsAction.fromVariantMap(variantMap["Hierarchy Construction"].toMap());
    _gradientDescentSettingsAction.fromVariantMap(variantMap["Gradient Descent Settings"].toMap());
    _knnSettingsAction.fromVariantMap(variantMap["Knn Settings"].toMap());
    _topLevelScaleAction.fromVariantMap(variantMap["HSNE Scale"].toMap());
    
    _knnParameters.setKnnAlgorithm(static_cast<hdi::dr::knn_library>(variantMap["KnnLibrary"].toInt()));
    _knnParameters.setKnnDistanceMetric(static_cast<hdi::dr::knn_distance_metric>(variantMap["KnnMetric"].toInt()));
    _knnParameters.setAnnoyNumChecks(variantMap["NumChecksAKNN"].toInt());
    _knnParameters.setAnnoyNumTrees(variantMap["NumChecksAKNN"].toInt());
    _knnParameters.setHNSWm(variantMap["NumChecksAKNN"].toInt());
    _knnParameters.setHNSWef(variantMap["NumChecksAKNN"].toInt());

    _hsneParameters.setNumScales(variantMap["NumScales"].toUInt());
    _hsneParameters.setSeed(variantMap["Seed"].toInt());
    _hsneParameters.setNumWalksForLandmarkSelection(variantMap["NumWalksForLandmarkSelection"].toInt());
    _hsneParameters.setNumWalksForLandmarkSelectionThreshold(variantMap["NumWalksForLandmarkSelectionThreshold"].toFloat());
    _hsneParameters.setRandomWalkLength(variantMap["RandomWalkLength"].toInt());
    _hsneParameters.setNumNearestNeighbors(variantMap["NumNearestNeighbors"].toInt());
    _hsneParameters.setNumWalksForAreaOfInfluence(variantMap["NumWalksForAreaOfInfluence"].toInt());
    _hsneParameters.setMinWalksRequired(variantMap["MinWalksRequired"].toInt());
    _hsneParameters.useMonteCarloSampling(variantMap["MonteCarloSampling"].toBool());
    _hsneParameters.useOutOfCoreComputation(variantMap["OutOfCoreComputation"].toBool());
    _hsneParameters.setSaveHierarchyToDisk(variantMap["SaveHierarchyToDisk"].toBool());

    _tsneParameters.setNumIterations(variantMap["NumIterations"].toInt());
    _tsneParameters.setExaggerationIter(variantMap["ExaggerationIter"].toInt());
    _tsneParameters.setExponentialDecayIter(variantMap["ExponentialDecayIter"].toInt());
    _tsneParameters.setNumDimensionsOutput(variantMap["NumDimensionsOutput"].toInt());
    _tsneParameters.setUpdateCore(variantMap["UpdateCore"].toInt());
}

QVariantMap HsneSettingsAction::toVariantMap() const
{
    QVariantMap variantMap = GroupAction::toVariantMap();

    _generalHsneSettingsAction.insertIntoVariantMap(variantMap);
    _hierarchyConstructionSettingsAction.insertIntoVariantMap(variantMap);
    _gradientDescentSettingsAction.insertIntoVariantMap(variantMap);
    _knnSettingsAction.insertIntoVariantMap(variantMap);
    _topLevelScaleAction.insertIntoVariantMap(variantMap);

    variantMap.insert({ { "KnnLibrary", QVariant::fromValue(static_cast<int>(_knnParameters.getKnnAlgorithm())) } });
    variantMap.insert({ { "KnnMetric", QVariant::fromValue(static_cast<int>(_knnParameters.getKnnDistanceMetric())) } });
    variantMap.insert({ { "AnnoyNumChecks", QVariant::fromValue(_knnParameters.getAnnoyNumChecks()) } });
    variantMap.insert({ { "AnnoyNumTrees", QVariant::fromValue(_knnParameters.getAnnoyNumTrees()) } });
    variantMap.insert({ { "HNSWm", QVariant::fromValue(_knnParameters.getHNSWm()) } });
    variantMap.insert({ { "HNSWef", QVariant::fromValue(_knnParameters.getHNSWef()) } });

    variantMap.insert({ { "NumScales", QVariant::fromValue(_hsneParameters.getNumScales()) } });
    variantMap.insert({ { "Seed", QVariant::fromValue(_hsneParameters.getSeed()) } });
    variantMap.insert({ { "NumWalksForLandmarkSelection", QVariant::fromValue(_hsneParameters.getNumWalksForLandmarkSelection()) } });
    variantMap.insert({ { "NumWalksForLandmarkSelectionThreshold", QVariant::fromValue(_hsneParameters.getNumWalksForLandmarkSelectionThreshold()) } });
    variantMap.insert({ { "RandomWalkLength", QVariant::fromValue(_hsneParameters.getRandomWalkLength()) } });
    variantMap.insert({ { "NumNearestNeighbors", QVariant::fromValue(_hsneParameters.getNumNearestNeighbors()) } });
    variantMap.insert({ { "NumWalksForAreaOfInfluence", QVariant::fromValue(_hsneParameters.getNumWalksForAreaOfInfluence()) } });
    variantMap.insert({ { "MinWalksRequired", QVariant::fromValue(_hsneParameters.getMinWalksRequired()) } });
    variantMap.insert({ { "MonteCarloSampling", QVariant::fromValue(_hsneParameters.useMonteCarloSampling()) } });
    variantMap.insert({ { "OutOfCoreComputation", QVariant::fromValue(_hsneParameters.useOutOfCoreComputation()) } });
    variantMap.insert({ { "SaveHierarchyToDisk", QVariant::fromValue(_hsneParameters.getSaveHierarchyToDisk()) } });

    variantMap.insert({ { "NumIterations", QVariant::fromValue(_tsneParameters.getNumIterations()) } });
    variantMap.insert({ { "ExaggerationIter", QVariant::fromValue(_tsneParameters.getExaggerationIter()) } });
    variantMap.insert({ { "ExponentialDecayIter", QVariant::fromValue(_tsneParameters.getExponentialDecayIter()) } });
    variantMap.insert({ { "NumDimensionsOutput", QVariant::fromValue(_tsneParameters.getNumDimensionsOutput()) } });
    variantMap.insert({ { "UpdateCore", QVariant::fromValue(_tsneParameters.getUpdateCore()) } });

    return variantMap;
}
