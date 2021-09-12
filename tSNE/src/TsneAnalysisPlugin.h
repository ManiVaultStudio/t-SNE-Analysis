#pragma once

#define no_init_all deprecated

#include <AnalysisPlugin.h>

#include "TsneAnalysis.h"
#include "TsneSettingsAction.h"
#include "DimensionSelectionAction.h"

using namespace hdps::plugin;

class TsneAnalysisPlugin : public QObject, public AnalysisPlugin
{
    Q_OBJECT
public:
    TsneAnalysisPlugin(const PluginFactory* factory);
    ~TsneAnalysisPlugin(void) override;

    void init() override;

    void onDataEvent(hdps::DataEvent* dataEvent);

    void startComputation();
    void continueComputation();
    void stopComputation();

    TsneSettingsAction& getGeneralSettingsAction() { return _tsneSettingsAction; }
    DimensionSelectionAction& getDimensionSelectionAction() { return _dimensionSelectionAction; }

protected:
    TsneAnalysis                _tsneAnalysis;                  /** TSNE analysis */
    TsneSettingsAction          _tsneSettingsAction;            /** TSNE settings action */
    DimensionSelectionAction    _dimensionSelectionAction;      /** Dimension selection settings action */
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

    /** Returns the plugin icon */
    QIcon getIcon() const override;

    AnalysisPlugin* produce() override;

    hdps::DataTypes supportedDataTypes() const override;
};
