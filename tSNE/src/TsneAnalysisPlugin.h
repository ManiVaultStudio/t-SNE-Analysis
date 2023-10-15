#pragma once

#define no_init_all deprecated

#include <actions/WidgetAction.h>

#include <AnalysisPlugin.h>

#include <ForegroundTask.h>

#include "TsneAnalysis.h"
#include "TsneSettingsAction.h"

using namespace mv::plugin;
using namespace mv::gui;

class TsneAnalysisPlugin : public AnalysisPlugin
{
    Q_OBJECT
public:
    TsneAnalysisPlugin(const PluginFactory* factory);
    ~TsneAnalysisPlugin(void) override;

    void init() override;

    void startComputation();
    void continueComputation();
    void stopComputation();

public: // Action getters

    TsneSettingsAction& getTsneSettingsAction() { return _tsneSettingsAction; }

protected:
    TsneAnalysis            _tsneAnalysis;                  /** TSNE analysis */
    TsneSettingsAction      _tsneSettingsAction;            /** TSNE settings action */
    mv::ForegroundTask      _computationPreparationTask;    /** Task for reporting computation preparation progress */
    mv::ForegroundTask      _computationTask;               /** Task for reporting computation progress */
};

class TsneAnalysisPluginFactory : public AnalysisPluginFactory
{
    Q_INTERFACES(mv::plugin::AnalysisPluginFactory mv::plugin::PluginFactory)
        Q_OBJECT
        Q_PLUGIN_METADATA(IID   "nl.tudelft.TsneAnalysisPlugin"
                          FILE  "TsneAnalysisPlugin.json")

public:
    TsneAnalysisPluginFactory(void) {}
    ~TsneAnalysisPluginFactory(void) override {}

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
