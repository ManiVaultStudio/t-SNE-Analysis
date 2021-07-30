#pragma once

#define no_init_all deprecated

#include <AnalysisPlugin.h>

#include "TsneAnalysis.h"
#include "GeneralSettingsAction.h"
#include "AdvancedSettingsAction.h"
#include "DimensionSelectionAction.h"
#include "ContextMenuAction.h"

class TsneSettingsWidget;

using namespace hdps::plugin;

class TsneAnalysisPlugin : public QObject, public AnalysisPlugin
{
    Q_OBJECT
public:
    TsneAnalysisPlugin();
    ~TsneAnalysisPlugin(void) override;

    void init() override;

    void onDataEvent(hdps::DataEvent* dataEvent);

    /** Returns the icon of this plugin */
    QIcon getIcon() const override;

    void startComputation(const bool& restart);
    void stopComputation();

    GeneralSettingsAction& getGeneralSettingsAction() { return _generalSettingsAction; }
    AdvancedSettingsAction& getAdvancedSettingsAction() { return _advancedSettingsAction; }
    DimensionSelectionAction& getDimensionsSettingsAction() { return _dimensionSelectionAction; }
    ContextMenuAction& getContextMenuAction() { return _contextMenuAction; }

    hdps::DataTypes supportedDataTypes() const override;

    TsneParameters& getTsneParameters();

protected:
    TsneAnalysis                _tsneAnalysis;                  /** TSNE analysis */
    TsneParameters              _tsneParameters;                /** TSNE analysis */
    GeneralSettingsAction       _generalSettingsAction;         /** General settings action */
    AdvancedSettingsAction      _advancedSettingsAction;        /** Advanced settings action */
    DimensionSelectionAction    _dimensionSelectionAction;      /** Dimension selection action */
    ContextMenuAction           _contextMenuAction;             /** Context menu action */

    friend class GeneralSettingsAction;
    friend class AdvancedSettingsAction;
    friend class DimensionSelectionAction;
};

class TsneAnalysisPluginFactory : public AnalysisPluginFactory
{
    Q_INTERFACES(hdps::plugin::AnalysisPluginFactory hdps::plugin::PluginFactory)
        Q_OBJECT
        Q_PLUGIN_METADATA(IID   "nl.tudelft.TsneAnalysisPlugin"
            FILE  "TsneAnalysisPlugin.json")

public:
    TsneAnalysisPluginFactory(void) {}
    ~TsneAnalysisPluginFactory(void) override {}

    AnalysisPlugin* produce() override;
};
