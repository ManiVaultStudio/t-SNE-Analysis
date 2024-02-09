#include "GeneralTsneSettingsAction.h"
#include "TsneSettingsAction.h"

using namespace mv::gui;

GeneralTsneSettingsAction::GeneralTsneSettingsAction(TsneSettingsAction& tsneSettingsAction) :
    GroupAction(&tsneSettingsAction, "TSNE", true),
    _tsneSettingsAction(tsneSettingsAction),
    _knnAlgorithmAction(this, "kNN Algorithm"),
    _distanceMetricAction(this, "Distance metric"),
    _perplexityAction(this, "Perplexity"),
    _computationAction(this),
    _reinitAction(this, "Reintialize instead of recompute", false),
    _saveProbDistAction(this, "Save analysis to projects", false)
{
    addAction(&_knnAlgorithmAction);
    addAction(&_distanceMetricAction);
    addAction(&_perplexityAction);
    
    _computationAction.addActions();

    addAction(&_reinitAction);
    addAction(&_saveProbDistAction);

    _knnAlgorithmAction.setDefaultWidgetFlags(OptionAction::ComboBox);
    _distanceMetricAction.setDefaultWidgetFlags(OptionAction::ComboBox);
    _perplexityAction.setDefaultWidgetFlags(IntegralAction::SpinBox | IntegralAction::Slider);

    _knnAlgorithmAction.initialize(QStringList({ "FLANN", "HNSW", "ANNOY" }), "FLANN");
    _distanceMetricAction.initialize(QStringList({ "Euclidean", "Cosine", "Inner Product", "Manhattan", "Hamming", "Dot" }), "Euclidean");
    _perplexityAction.initialize(2, 50, 30);

    _reinitAction.setToolTip("Instead of recomputing knn, simple re-initialize t-SNE embedding and recompute gradient descent.");
    _saveProbDistAction.setToolTip("When saving the t-SNE analysis with your project, you can compute additional iterations without recomputing similarities from scratch.");

    const auto updateKnnAlgorithm = [this]() -> void {
        if (_knnAlgorithmAction.getCurrentText() == "FLANN")
            _tsneSettingsAction.getKnnParameters().setKnnAlgorithm(hdi::dr::knn_library::KNN_FLANN);

        if (_knnAlgorithmAction.getCurrentText() == "HNSW")
            _tsneSettingsAction.getKnnParameters().setKnnAlgorithm(hdi::dr::knn_library::KNN_HNSW);

        if (_knnAlgorithmAction.getCurrentText() == "ANNOY")
            _tsneSettingsAction.getKnnParameters().setKnnAlgorithm(hdi::dr::knn_library::KNN_ANNOY);
    };

    const auto updateDistanceMetric = [this]() -> void {
        if (_distanceMetricAction.getCurrentText() == "Euclidean")
            _tsneSettingsAction.getKnnParameters().setKnnDistanceMetric(hdi::dr::knn_distance_metric::KNN_METRIC_EUCLIDEAN);

        if (_distanceMetricAction.getCurrentText() == "Cosine")
            _tsneSettingsAction.getKnnParameters().setKnnDistanceMetric(hdi::dr::knn_distance_metric::KNN_METRIC_COSINE);

        if (_distanceMetricAction.getCurrentText() == "Inner Product")
            _tsneSettingsAction.getKnnParameters().setKnnDistanceMetric(hdi::dr::knn_distance_metric::KNN_METRIC_INNER_PRODUCT);

        if (_distanceMetricAction.getCurrentText() == "Manhattan")
            _tsneSettingsAction.getKnnParameters().setKnnDistanceMetric(hdi::dr::knn_distance_metric::KNN_METRIC_MANHATTAN);

        if (_distanceMetricAction.getCurrentText() == "Hamming")
            _tsneSettingsAction.getKnnParameters().setKnnDistanceMetric(hdi::dr::knn_distance_metric::KNN_METRIC_HAMMING);

        if (_distanceMetricAction.getCurrentText() == "Dot")
            _tsneSettingsAction.getKnnParameters().setKnnDistanceMetric(hdi::dr::knn_distance_metric::KNN_METRIC_DOT);
    };

    const auto updateNumIterations = [this]() -> void {
        _tsneSettingsAction.getTsneParameters().setNumIterations(_computationAction.getNumIterationsAction().getValue());
    };

    const auto updatePerplexity = [this]() -> void {
        _tsneSettingsAction.getTsneParameters().setPerplexity(_perplexityAction.getValue());
    };

    const auto updateCoreUpdate = [this]() -> void {
        _tsneSettingsAction.getTsneParameters().setUpdateCore(_computationAction.getUpdateIterationsAction().getValue());
    };

    // currently unused
    //const auto isResettable = [this]() -> bool {
    //    if (_knnAlgorithmAction.isResettable())
    //        return true;

    //    if (_distanceMetricAction.isResettable())
    //        return true;

    //    if (_computationAction.getNumIterationsAction().isResettable())
    //        return true;

    //    if (_computationAction.getUpdateIterationsAction().isResettable())
    //        return true;

    //    if (_perplexityAction.isResettable())
    //        return true;

    //    return false;
    //};

    const auto updateReadOnly = [this]() -> void {
        const auto enable = !isReadOnly();

        _knnAlgorithmAction.setEnabled(enable);
        _distanceMetricAction.setEnabled(enable);
        _computationAction.getNumIterationsAction().setEnabled(enable);
        _perplexityAction.setEnabled(enable);
        _computationAction.getUpdateIterationsAction().setEnabled(enable);
        _reinitAction.setEnabled(enable);
        _saveProbDistAction.setEnabled(enable);
    };

    connect(&_knnAlgorithmAction, &OptionAction::currentIndexChanged, this, [this, updateKnnAlgorithm](const std::int32_t& currentIndex) {
        updateKnnAlgorithm();
    });

    connect(&_distanceMetricAction, &OptionAction::currentIndexChanged, this, [this, updateDistanceMetric](const std::int32_t& currentIndex) {
        updateDistanceMetric();
    });

    connect(&_computationAction.getNumIterationsAction(), &IntegralAction::valueChanged, this, [this, updateNumIterations](const std::int32_t& value) {
        updateNumIterations();
    });

    connect(&_perplexityAction, &IntegralAction::valueChanged, this, [this, updatePerplexity](const std::int32_t& value) {
        updatePerplexity();
    });

    connect(&_computationAction.getUpdateIterationsAction(), &IntegralAction::valueChanged, this, [this, updateCoreUpdate](const std::int32_t& value) {
        updateCoreUpdate();
    });

    connect(&_reinitAction, &ToggleAction::toggled, this, [this, updateCoreUpdate](const bool toggled) {
        QString newText = (toggled) ? "Reinit" : "Start";
        _computationAction.getStartComputationAction().setText(newText);
        });

    connect(this, &GroupAction::readOnlyChanged, this, [this, updateReadOnly](const bool& readOnly) {
        updateReadOnly();
    });

    updateKnnAlgorithm();
    updateDistanceMetric();
    updateNumIterations();
    updatePerplexity();
    updateCoreUpdate();
    updateReadOnly();
}

void GeneralTsneSettingsAction::fromVariantMap(const QVariantMap& variantMap)
{
    GroupAction::fromVariantMap(variantMap);

    _knnAlgorithmAction.fromParentVariantMap(variantMap);
    _distanceMetricAction.fromParentVariantMap(variantMap);
    _perplexityAction.fromParentVariantMap(variantMap);
    _computationAction.fromParentVariantMap(variantMap);
    _reinitAction.fromParentVariantMap(variantMap);
    _saveProbDistAction.fromParentVariantMap(variantMap);
}

QVariantMap GeneralTsneSettingsAction::toVariantMap() const
{
    qDebug() << "GeneralTsneSettingsAction::toVariantMap:";

    QVariantMap variantMap = GroupAction::toVariantMap();

    _knnAlgorithmAction.insertIntoVariantMap(variantMap);
    _distanceMetricAction.insertIntoVariantMap(variantMap);
    _perplexityAction.insertIntoVariantMap(variantMap);
    _computationAction.insertIntoVariantMap(variantMap);
    _reinitAction.insertIntoVariantMap(variantMap);
    _saveProbDistAction.insertIntoVariantMap(variantMap);

    return variantMap;
}
