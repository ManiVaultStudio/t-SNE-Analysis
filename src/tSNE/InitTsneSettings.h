#pragma once

#include "actions/DatasetPickerAction.h"
#include "actions/DecimalAction.h"
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

public: // Action getters

    TsneSettingsAction& getTsneSettingsAction() { return _tsneSettingsAction; };
    ToggleAction& getRandomInitAction() { return _randomInitAction; };
    IntegralAction& getRandomSeedAction() { return _randomSeedAction; };
    DatasetPickerAction& getDataSetInitAction() { return _dataSetInitAction; };
    DimensionPickerAction& getDataDimensionAction() { return _dataDimensionAction; };
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
    IntegralAction          _randomSeedAction;              /** Random seed for init */
    DatasetPickerAction     _dataSetInitAction;             /** Data set to use for init */
    DimensionPickerAction   _dataDimensionAction;           /** Dimensions of dataset to use for init */
    ToggleAction            _rescaleInitAction;             /** Whether to rescale the init embedding */
};
