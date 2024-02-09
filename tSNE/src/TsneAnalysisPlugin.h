#pragma once

#include <AnalysisPlugin.h>
#include <Task.h>

#include "TsneAnalysis.h"

using namespace mv::plugin;
using namespace mv::gui;

class TsneSettingsAction;

class TsneAnalysisPlugin : public AnalysisPlugin
{
    Q_OBJECT
public:
    TsneAnalysisPlugin(const PluginFactory* factory);
    ~TsneAnalysisPlugin(void) override;

    void init() override;

    void startComputation();
    void reinitializeComputation();
    void continueComputation();
    void stopComputation();

public: // Serialization

    /**
     * Load plugin from variant map
     * @param Variant map representation of the plugin
     */
    Q_INVOKABLE void fromVariantMap(const QVariantMap& variantMap) override;

    /**
     * Save plugin to variant map
     * @return Variant map representation of the plugin
     */
    Q_INVOKABLE QVariantMap toVariantMap() const override;

private:
    TsneAnalysis                        _tsneAnalysis;          /** TSNE analysis */
    TsneSettingsAction*                 _tsneSettingsAction;    /** TSNE settings action */
    mv::Task                            _dataPreparationTask;   /** Task for reporting data preparation progress */

private:
    ProbDistMatrix                      _probDistMatrix;        /** Probability distribution matrix used for serialization */
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
