#pragma once

#include <AnalysisPlugin.h>

#include "HsneHierarchy.h"
#include "TsneAnalysis.h"
#include "HsneSettingsAction.h"

using namespace hdps::plugin;
using namespace hdps::gui;

class HsneScaleAction;

/**
 * HsneAnalysisPlugin
 *
 * Main plugin class responsible for computing HSNE hierarchies over point data
 *
 * @author Julian Thijssen
 */
class HsneAnalysisPlugin : public AnalysisPlugin
{
    Q_OBJECT
public:
    HsneAnalysisPlugin(const PluginFactory* factory);
    ~HsneAnalysisPlugin() override;

    void init() override;

    void computeTopLevelEmbedding();

    HsneHierarchy& getHierarchy() { return _hierarchy; }
    TsneAnalysis& getTsneAnalysis() { return _tsneAnalysis; }

    HsneSettingsAction& getHsneSettingsAction() { return *_hsneSettingsAction; }

private:
    HsneHierarchy           _hierarchy;                 /** HSNE hierarchy */
    TsneAnalysis            _tsneAnalysis;              /** TSNE analysis */
    HsneSettingsAction*     _hsneSettingsAction;        /** Pointer to HSNE settings action */
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

    /** Returns the plugin icon */
    QIcon getIcon() const override;

    AnalysisPlugin* produce() override;

    hdps::DataTypes supportedDataTypes() const override;
};
