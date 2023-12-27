#pragma once

#include "TsneParameters.h"
#include "GeneralTsneSettingsAction.h"
#include "AdvancedTsneSettingsAction.h"

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
    TsneSettingsAction(QObject* parent);

    /**
     * Get the context menu for the action
     * @param parent Parent widget
     * @return Context menu
     */
    QMenu* getContextMenu(QWidget* parent = nullptr) override;

public: // Action getters

    TsneParameters& getTsneParameters() { return _tsneParameters; }
    GeneralTsneSettingsAction& getGeneralTsneSettingsAction() { return _generalTsneSettingsAction; }
    AdvancedTsneSettingsAction& getAdvancedTsneSettingsAction() { return _advancedTsneSettingsAction; }
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

protected:
    TsneParameters                  _tsneParameters;                /** TSNE parameters */
    GeneralTsneSettingsAction       _generalTsneSettingsAction;     /** General tSNE settings action */
    AdvancedTsneSettingsAction      _advancedTsneSettingsAction;    /** Advanced tSNE settings action */

    friend class Widget;
};