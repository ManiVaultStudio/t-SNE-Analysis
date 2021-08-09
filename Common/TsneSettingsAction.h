#pragma once

#include "TsneParameters.h"
#include "TsneComputationAction.h"
#include "GeneralTsneSettingsAction.h"
#include "AdvancedTsneSettingsAction.h"

class QMenu;

/**
 * TSNE settings class
 *
 * Settings actions class for general/advanced HSNE/TSNE settings
 *
 * @author Thomas Kroes
 */
class TsneSettingsAction : public hdps::gui::WidgetActionGroup
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
    TsneComputationAction& getComputationAction() { return _computationAction; }
    GeneralTsneSettingsAction& getGeneralTsneSettingsAction() { return _generalTsneSettingsAction; }
    AdvancedTsneSettingsAction& getAdvancedTsneSettingsAction() { return _advancedTsneSettingsAction; }

protected:
    TsneParameters                  _tsneParameters;                /** TSNE parameters */
    TsneComputationAction           _computationAction;             /** Computation action */
    GeneralTsneSettingsAction       _generalTsneSettingsAction;     /** General tSNE settings action */
    AdvancedTsneSettingsAction      _advancedTsneSettingsAction;    /** Advanced tSNE settings action */

    friend class Widget;
};