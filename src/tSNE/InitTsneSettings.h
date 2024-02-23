#pragma once

#include "actions/DatasetPickerAction.h"
#include "actions/IntegralAction.h"
#include "actions/ToggleAction.h"
#include "PointData/DimensionPickerAction.h"

#include <vector>

using namespace mv::gui;

class TsneSettingsAction;

/**
 * Init TSNE setting action class
 *
 * Setup of t-SNE embedding initialization
 *
 * @author Alexander Vieth
 */
class InitTsneSettings : public GroupAction
{
public:

    /**
     * Constructor
     * @param tsneSettingsAction Reference to TSNE settings action
     */
    InitTsneSettings(TsneSettingsAction& tsneSettingsAction);

    std::vector<float> getInitEmbedding(size_t numPoints);

    void updateSeed();

    /**
     * only list point datasets with at least 2 dimensions
     * and the same number of points as the input data
    */
    void updateDataPicker(size_t numPointsInputData);

public: // Action getters

    TsneSettingsAction& getTsneSettingsAction() { return _tsneSettingsAction; };
    ToggleAction& getRandomInitAction() { return _randomInitAction; };
    ToggleAction& getNewRandomSeedAction() { return _newRandomSeedAction; };
    IntegralAction& getRandomSeedAction() { return _randomSeedAction; };
    DatasetPickerAction& getDatasetInitAction() { return _datasetInitAction; };
    DimensionPickerAction& getDataDimensionXAction() { return _dataDimensionActionX; };
    DimensionPickerAction& getDataDimensionYAction() { return _dataDimensionActionY; };
    ToggleAction& getRescaleInitAction() { return _rescaleInitAction; }

public: // Serialization

    /**
     * Load plugin from variant map
     * @param Variant map representation of the plugin
     */
    void fromVariantMap(const QVariantMap& variantMap) override;

    /**
     * Save plugin to variant map
     * @return Variant map representation of the plugin
     */
    QVariantMap toVariantMap() const override;

protected:
    TsneSettingsAction&     _tsneSettingsAction;            /** Reference to parent tSNE settings action */
    ToggleAction            _randomInitAction;              /** Init t-SNE randomly */
    ToggleAction            _newRandomSeedAction;                 /** New random seed on re-init */
    IntegralAction          _randomSeedAction;              /** Random seed for init */
    DatasetPickerAction     _datasetInitAction;             /** Data set to use for init */
    DimensionPickerAction   _dataDimensionActionX;          /** Dimension of dataset to use for init X dim */
    DimensionPickerAction   _dataDimensionActionY;          /** Dimension of dataset to use for init Y dim */
    ToggleAction            _rescaleInitAction;             /** Whether to rescale the init embedding */

private:
    size_t                  _numPointsInputData;            /** Number of points of the input dataset */
};
