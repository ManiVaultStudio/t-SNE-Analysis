#pragma once

#include <AnalysisPlugin.h>

#include "HsneHierarchy.h"
#include "TsneAnalysis.h"
#include "HsneSettingsAction.h"

using namespace hdps::plugin;
using namespace hdps::gui;

class HsneAnalysisPlugin : public QObject, public AnalysisPlugin
{
    Q_OBJECT
public:
    HsneAnalysisPlugin();
    ~HsneAnalysisPlugin() override;

    void init() override;

    void computeTopLevelEmbedding();

    /** Returns the icon of this plugin */
    QIcon getIcon() const override;

private:
    HsneHierarchy       _hierarchy;                 /** HSNE hierarchy */
    TsneAnalysis        _tsneAnalysis;              /** TSNE analysis */
    HsneSettingsAction  _hsneSettingsAction;        /** HSNE settings action */
};

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

    hdps::DataTypes supportedDataTypes() const override;
};
