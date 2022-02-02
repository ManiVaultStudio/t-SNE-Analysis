#pragma once

#define no_init_all deprecated

#include <actions/WidgetAction.h>

#include <AnalysisPlugin.h>

#include "TsneAnalysis.h"
#include "TsneSettingsAction.h"

using namespace hdps::plugin;
using namespace hdps::gui;

class TsneAnalysisPlugin : public WidgetAction, public AnalysisPlugin
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

public: // Action getters

    TsneSettingsAction& getTsneSettingsAction() { return _tsneSettingsAction; }

protected:
    TsneAnalysis        _tsneAnalysis;          /** TSNE analysis */
    TsneSettingsAction  _tsneSettingsAction;    /** TSNE settings action */
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
