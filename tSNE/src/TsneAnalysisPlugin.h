#pragma once

#define no_init_all deprecated

#include <AnalysisPlugin.h>

#include "TsneAnalysis.h"
#include "SettingsAction.h"

class TsneSettingsWidget;

using namespace hdps::plugin;

// =============================================================================
// View
// =============================================================================

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

    void startComputation();
    void stopComputation();

public slots:
    void dataSetPicked(const QString& name);
    void onKnnAlgorithmPicked(const int index);
    void onDistanceMetricPicked(const int index);
    void onNewEmbedding();

private:
    void initializeTsne();

    TsneAnalysis	_tsne;
	SettingsAction	_settingsAction;
};

// =============================================================================
// Factory
// =============================================================================

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
