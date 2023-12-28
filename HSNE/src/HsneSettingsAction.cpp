#include "HsneSettingsAction.h"
#include "HsneAnalysisPlugin.h"

using namespace mv::gui;

HsneSettingsAction::HsneSettingsAction(HsneAnalysisPlugin* hsneAnalysisPlugin) :
    GroupAction(hsneAnalysisPlugin, "HSNE", true),
    _hsneAnalysisPlugin(hsneAnalysisPlugin),
    _hsneParameters(),
    _generalHsneSettingsAction(*this),
    _advancedHsneSettingsAction(*this),
    _topLevelScaleAction(this, _tsneSettingsAction, hsneAnalysisPlugin->getHierarchy(), hsneAnalysisPlugin->getInputDataset<Points>(), hsneAnalysisPlugin->getOutputDataset<Points>()),
    _tsneSettingsAction(this)
{
    setText("HSNE");
    setSerializationName("hsneSettings");
    setObjectName("Settings");

    _tsneSettingsAction.setObjectName("TSNE");

    const auto updateReadOnly = [this]() -> void {
        _generalHsneSettingsAction.setReadOnly(isReadOnly());
        _advancedHsneSettingsAction.setReadOnly(isReadOnly());
        _topLevelScaleAction.setReadOnly(isReadOnly());
        _tsneSettingsAction.setReadOnly(isReadOnly());
    };

    const auto updateDistanceMetric = [this]() -> void {
        auto currentText = _tsneSettingsAction.getGeneralTsneSettingsAction().getDistanceMetricAction().getCurrentText();
        auto metric = hdi::dr::knn_distance_metric::KNN_METRIC_EUCLIDEAN;

        if (currentText == "Euclidean")
            metric = hdi::dr::knn_distance_metric::KNN_METRIC_EUCLIDEAN;

        if (currentText == "Cosine")
            metric = hdi::dr::knn_distance_metric::KNN_METRIC_COSINE;

        if (currentText == "Inner Product")
            metric = hdi::dr::knn_distance_metric::KNN_METRIC_INNER_PRODUCT;

        if (currentText == "Manhattan")
            metric = hdi::dr::knn_distance_metric::KNN_METRIC_MANHATTAN;

        if (currentText == "Hamming")
            metric = hdi::dr::knn_distance_metric::KNN_METRIC_HAMMING;

        if (currentText == "Dot")
            metric = hdi::dr::knn_distance_metric::KNN_METRIC_DOT;

        _generalHsneSettingsAction.setDistanceMetric(metric);

    };


    connect(this, &GroupAction::readOnlyChanged, this, [this, updateReadOnly](const bool& readOnly) {
        updateReadOnly();
    });

    // Use perplexity as set t-SNE UI 
    connect(&_tsneSettingsAction.getGeneralTsneSettingsAction().getPerplexityAction(), &IntegralAction::valueChanged, &_generalHsneSettingsAction, &GeneralHsneSettingsAction::setPerplexity);
    _generalHsneSettingsAction.setPerplexity(_tsneSettingsAction.getGeneralTsneSettingsAction().getPerplexityAction().getValue());

    // Use metric as set in t-SNE UI
    connect(&_tsneSettingsAction.getGeneralTsneSettingsAction().getDistanceMetricAction(), &OptionAction::currentIndexChanged, this, [this, updateDistanceMetric](const std::int32_t& currentIndex) {
        updateDistanceMetric();
        });

    updateReadOnly();
    updateDistanceMetric();
}

HsneParameters& HsneSettingsAction::getHsneParameters()
{
    return _hsneParameters;
}

TsneParameters& HsneSettingsAction::getTsneParameters()
{
    return _tsneSettingsAction.getTsneParameters();
}

void HsneSettingsAction::fromVariantMap(const QVariantMap& variantMap)
{
    GroupAction::fromVariantMap(variantMap);

    variantMapMustContain(variantMap, "generalHsneSettings");
    variantMapMustContain(variantMap, "advancedHsneSettings");
    variantMapMustContain(variantMap, "tsneSettings");
    variantMapMustContain(variantMap, "topLevelScale");
    variantMapMustContain(variantMap, "hsneParameters");

    _generalHsneSettingsAction.fromVariantMap(variantMap["generalHsneSettings"].toMap());
    _advancedHsneSettingsAction.fromVariantMap(variantMap["advancedHsneSettings"].toMap());
    _tsneSettingsAction.fromVariantMap(variantMap["tsneSettings"].toMap());
    _topLevelScaleAction.fromVariantMap(variantMap["topLevelScale"].toMap());
    
    _hsneParameters.setKnnLibrary(static_cast<hdi::dr::knn_library>(variantMap["KnnLibrary"].toInt()));
    _hsneParameters.setKnnMetric(static_cast<hdi::dr::knn_distance_metric>(variantMap["KnnMetric"].toInt()));
    _hsneParameters.setNumScales(variantMap["NumScales"].toUInt());
    _hsneParameters.setSeed(variantMap["Seed"].toInt());
    _hsneParameters.setNumWalksForLandmarkSelection(variantMap["NumWalksForLandmarkSelection"].toInt());
    _hsneParameters.setNumWalksForLandmarkSelectionThreshold(variantMap["NumWalksForLandmarkSelectionThreshold"].toFloat());

    _hsneParameters.setRandomWalkLength(variantMap["RandomWalkLength"].toInt());
    _hsneParameters.setNumNearestNeighbors(variantMap["NumNearestNeighbors"].toInt());
    _hsneParameters.setNumWalksForAreaOfInfluence(variantMap["NumWalksForAreaOfInfluence"].toInt());
    _hsneParameters.setMinWalksRequired(variantMap["MinWalksRequired"].toInt());
    _hsneParameters.setNumChecksAKNN(variantMap["NumChecksAKNN"].toInt());
    _hsneParameters.useMonteCarloSampling(variantMap["MonteCarloSampling"].toBool());
    _hsneParameters.useOutOfCoreComputation(variantMap["OutOfCoreComputation"].toBool());
    _hsneParameters.setSaveHierarchyToDisk(variantMap["SaveHierarchyToDisk"].toBool());
}

QVariantMap HsneSettingsAction::toVariantMap() const
{
    qDebug() << "HsneSettingsAction::toVariantMap:";

    QVariantMap variantMap = GroupAction::toVariantMap();

    _generalHsneSettingsAction.insertIntoVariantMap(variantMap);
    _advancedHsneSettingsAction.insertIntoVariantMap(variantMap);
    _topLevelScaleAction.insertIntoVariantMap(variantMap);
    _tsneSettingsAction.insertIntoVariantMap(variantMap);

    QVariantMap settingsMap = { { "hsneParameters", QUuid::createUuid().toString(QUuid::WithoutBraces) } };

    variantMap.insert({ { "KnnLibrary", QVariant::fromValue(static_cast<int>(_hsneParameters.getKnnLibrary())) } });
    variantMap.insert({ { "KnnMetric", QVariant::fromValue(static_cast<int>(_hsneParameters.getKnnMetric())) } });
    variantMap.insert({ { "NumScales", QVariant::fromValue(_hsneParameters.getNumScales()) } });
    variantMap.insert({ { "Seed", QVariant::fromValue(_hsneParameters.getSeed()) } });
    variantMap.insert({ { "NumWalksForLandmarkSelection", QVariant::fromValue(_hsneParameters.getNumWalksForLandmarkSelection()) } });
    variantMap.insert({ { "NumWalksForLandmarkSelectionThreshold", QVariant::fromValue(_hsneParameters.getNumWalksForLandmarkSelectionThreshold()) } });

    variantMap.insert({ { "RandomWalkLength", QVariant::fromValue(_hsneParameters.getRandomWalkLength()) } });
    variantMap.insert({ { "NumNearestNeighbors", QVariant::fromValue(_hsneParameters.getNumNearestNeighbors()) } });
    variantMap.insert({ { "NumWalksForAreaOfInfluence", QVariant::fromValue(_hsneParameters.getNumWalksForAreaOfInfluence()) } });
    variantMap.insert({ { "MinWalksRequired", QVariant::fromValue(_hsneParameters.getMinWalksRequired()) } });
    variantMap.insert({ { "NumChecksAKNN", QVariant::fromValue(_hsneParameters.getNumChecksAKNN()) } });
    variantMap.insert({ { "MonteCarloSampling", QVariant::fromValue(_hsneParameters.useMonteCarloSampling()) } });
    variantMap.insert({ { "OutOfCoreComputation", QVariant::fromValue(_hsneParameters.useOutOfCoreComputation()) } });
    variantMap.insert({ { "SaveHierarchyToDisk", QVariant::fromValue(_hsneParameters.getSaveHierarchyToDisk()) } });

    return variantMap;
}
