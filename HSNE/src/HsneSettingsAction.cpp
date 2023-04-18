#include "HsneSettingsAction.h"
#include "HsneAnalysisPlugin.h"

using namespace hdps::gui;

HsneSettingsAction::HsneSettingsAction(HsneAnalysisPlugin* hsneAnalysisPlugin) :
    GroupAction(hsneAnalysisPlugin, true),
    _hsneAnalysisPlugin(hsneAnalysisPlugin),
    _hsneParameters(),
    _tsneParameters(),
    _generalHsneSettingsAction(*this),
    _advancedHsneSettingsAction(*this),
    _topLevelScaleAction(this, _tsneSettingsAction, hsneAnalysisPlugin->getHierarchy(), hsneAnalysisPlugin->getInputDataset<Points>(), hsneAnalysisPlugin->getOutputDataset<Points>()),
    _tsneSettingsAction(this)
{
    setText("HSNE");
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
    return _tsneParameters;
}
