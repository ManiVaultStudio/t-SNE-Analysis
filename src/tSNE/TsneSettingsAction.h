#pragma once

#include "GeneralTsneSettingsAction.h"
#include "GradientDescentSettingsAction.h"
#include "InitTsneSettings.h"
#include "KnnParameters.h"
#include "KnnSettingsAction.h"
#include "TsneParameters.h"

using namespace mv::gui;

class QMenu;

class TsneComputationAction;

/**
 * TSNE settings class
 *
 * Settings actions class for general/advanced HSNE/TSNE settings
 *
 * @author Thomas Kroes
 */
class TsneSettingsAction : public GroupAction
{
public:

    /**
     * Constructor
     * @param parent Pointer to parent object
     */
    TsneSettingsAction(QObject* parent, size_t numPointsInputData);

    /**
     * Get the context menu for the action
     * @param parent Parent widget
     * @return Context menu
     */
    QMenu* getContextMenu(QWidget* parent = nullptr) override;

    TsneParameters& getTsneParameters() { return _tsneParameters; }
    KnnParameters& getKnnParameters() { return _knnParameters; }

public: // Action getters

    GeneralTsneSettingsAction& getGeneralTsneSettingsAction() { return _generalTsneSettingsAction; }
    InitTsneSettings& getInitalEmbeddingSettingsAction() { return _initTsneSettingsAction; }
    GradientDescentSettingsAction& getGradientDescentSettingsAction() { return _gradientDescentSettingsAction; }
    KnnSettingsAction& getKnnSettingsAction() { return _knnSettingsAction; }
    TsneComputationAction& getComputationAction() { return _generalTsneSettingsAction.getComputationAction(); }

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

private:
    TsneParameters                  _tsneParameters;                    /** TSNE parameters */
    KnnParameters                   _knnParameters;                     /** Knn parameters */

private:
    GeneralTsneSettingsAction       _generalTsneSettingsAction;         /** General tSNE settings action */
    InitTsneSettings                _initTsneSettingsAction;            /** Inital embedding settings action */
    GradientDescentSettingsAction   _gradientDescentSettingsAction;     /** Gradient descent settings action */
    KnnSettingsAction               _knnSettingsAction;                 /** knn settings action */

};
