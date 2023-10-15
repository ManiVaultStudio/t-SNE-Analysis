#pragma once

#include <AnalysisPlugin.h>

#include "HsneHierarchy.h"
#include "TsneAnalysis.h"
#include "HsneSettingsAction.h"

using namespace mv::plugin;
using namespace mv::gui;

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
    void continueComputation();

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
    Q_INTERFACES(mv::plugin::AnalysisPluginFactory mv::plugin::PluginFactory)
        Q_OBJECT
        Q_PLUGIN_METADATA(IID   "nl.tudelft.HsneAnalysisPlugin"
                          FILE  "HsneAnalysisPlugin.json")

public:
    HsneAnalysisPluginFactory(void) {}
    ~HsneAnalysisPluginFactory(void) override {}

    /**
     * Get plugin icon
     * @param color Icon color for flat (font) icons
     * @return Icon
     */
    QIcon getIcon(const QColor& color = Qt::black) const override;

    /**
     * Produces the plugin
     * @return Pointer to the produced plugin
     */
    AnalysisPlugin* produce() override;

    /**
     * Get plugin trigger actions given \p datasets
     * @param datasets Vector of input datasets
     * @return Vector of plugin trigger actions
     */
    PluginTriggerActions getPluginTriggerActions(const mv::Datasets& datasets) const override;
};
