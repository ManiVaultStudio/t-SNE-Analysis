#pragma once

#define no_init_all deprecated

#include <AnalysisPlugin.h>

#include "TsneAnalysis.h"
#include "SettingsAction.h"

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

protected:
    TsneAnalysis	_tsne;
	SettingsAction	_settingsAction;

	friend class SettingsAction;
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
