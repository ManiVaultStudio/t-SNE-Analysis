#pragma once

#include <AnalysisPlugin.h>

#include "TsneAnalysis.h"
#include "HsneAnalysis.h"
#include "HsneSettingsWidget.h"

using namespace hdps::plugin;
using namespace hdps::gui;

// =============================================================================
// View
// =============================================================================

class HsneAnalysisPlugin : public QObject, public AnalysisPlugin
{
    Q_OBJECT
public:
    HsneAnalysisPlugin();
    ~HsneAnalysisPlugin(void) override;

    void init() override;

    /**
     * This function is called from the core system whenever data is added.
     *
     * @param name The name of the added dataset
     */
    void onDataEvent(hdps::DataEvent* dataEvent);

public slots:
    void dataSetPicked(const QString& name);
    void startComputation();

private:
    HsneAnalysis _hsne;

    std::unique_ptr<HsneSettingsWidget> _settings;
};

// =============================================================================
// Factory
// =============================================================================

class HsneAnalysisPluginFactory : public AnalysisPluginFactory
{
    Q_INTERFACES(hdps::plugin::AnalysisPluginFactory hdps::plugin::PluginFactory)
        Q_OBJECT
        Q_PLUGIN_METADATA(IID   "nl.tudelft.HsneAnalysisPlugin"
            FILE  "HsneAnalysisPlugin.json")

public:
    HsneAnalysisPluginFactory(void) {}
    ~HsneAnalysisPluginFactory(void) override {}

    AnalysisPlugin* produce() override;
};
