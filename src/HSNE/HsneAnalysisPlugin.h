#pragma once

#include <AnalysisPlugin.h>

#include <event/EventListener.h>

#include "HsneHierarchy.h"
#include "HsneSettingsAction.h"
#include "TsneAnalysis.h"

#include <QPointer>
#include <QUrl>

using namespace mv::plugin;
using namespace mv::gui;

class HsneScaleAction;

namespace mv::util {
    class MarkdownDialog;
}

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
    void removePreviouslyCreatedDatasets();

    HsneHierarchy& getHierarchy() { return *_hierarchy.get(); }
    TsneAnalysis& getTsneAnalysis() { return _tsneAnalysis; }

    HsneSettingsAction& getHsneSettingsAction() { return *_hsneSettingsAction; }

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

signals:
    // Local signals
    void startHierarchyWorker();

private:
    std::unique_ptr<HsneHierarchy> _hierarchy;      /** HSNE hierarchy */
    QThread                 _hierarchyThread;       /** Qt Thread for managing HSNE hierarchy computation */
    TsneAnalysis            _tsneAnalysis;          /** TSNE analysis */
    HsneSettingsAction*     _hsneSettingsAction;    /** Pointer to HSNE settings action */
    EventListener           _eventListener;         /** Listen to ManiVault events */
    mv::Dataset<Points>     _selectionHelperData;   /** Invisible selection helper dataset */
};

class HsneAnalysisPluginFactory : public AnalysisPluginFactory
{
    Q_INTERFACES(mv::plugin::AnalysisPluginFactory mv::plugin::PluginFactory)
        Q_OBJECT
        Q_PLUGIN_METADATA(IID   "studio.manivault.HsneAnalysisPlugin"
                          FILE  "PluginInfo.json")

public:
    HsneAnalysisPluginFactory();

    ~HsneAnalysisPluginFactory() override {}

    mv::DataTypes supportedDataTypes() const override;

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

    QUrl getReadmeMarkdownUrl() const override;

    bool hasHelp() const override { return true; }

    QUrl getRepositoryUrl() const override;

private:
    QPointer<mv::util::MarkdownDialog>   _helpMarkdownDialog = {};
};
