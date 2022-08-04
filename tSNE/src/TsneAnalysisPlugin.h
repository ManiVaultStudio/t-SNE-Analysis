#pragma once

#define no_init_all deprecated

#include <actions/WidgetAction.h>

#include <AnalysisPlugin.h>

#include "TsneAnalysis.h"
#include "TsneSettingsAction.h"

using namespace hdps::plugin;
using namespace hdps::gui;

class TsneAnalysisPlugin : public AnalysisPlugin
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
    PluginTriggerActions getPluginTriggerActions(const hdps::Datasets& datasets) const override;
};
