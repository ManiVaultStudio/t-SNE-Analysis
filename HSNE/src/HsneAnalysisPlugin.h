#pragma once

#include <AnalysisPlugin.h>

#include "HsneHierarchy.h"
#include "TsneAnalysis.h"
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
    HsneAnalysisPlugin() : AnalysisPlugin("H-SNE Analysis") { }
    ~HsneAnalysisPlugin(void) override;

    void init() override;

    /**
     * This function is called from the core system whenever data is added.
     *
     * @param name The name of the added dataset
     */
    void dataAdded(const QString name) Q_DECL_OVERRIDE;

    /**
     * This function is called from the core system whenever data is altered.
     *
     * @param name The name of the altered dataset
     */
    void dataChanged(const QString name) Q_DECL_OVERRIDE;

    /**
     * This function is called from the core system whenever data is removed.
     *
     * @param name The name of the removed dataset
     */
    void dataRemoved(const QString name) Q_DECL_OVERRIDE;

    /**
     * Unused in this plugin
     */
    void selectionChanged(const QString dataName) Q_DECL_OVERRIDE;

    /**
     * Returns a list of data types supported by this plugin.
     */
    hdps::DataTypes supportedDataTypes() const Q_DECL_OVERRIDE;

    void computeEmbedding();
    void drillIn(QString embeddingName);
    SettingsWidget* const getSettings() override;

public slots:
    void dataSetPicked(const QString& name);
    void startComputation();
    void onDrillIn();
    void onNewEmbedding();

public: // GUI
    /**
     * Generates a context menu for display in other (view) plugins
     * @param kind Kind of plugin in which the context menu will be shown
     * @return Context menu
     */
    QMenu* contextMenu(const QVariant& context) override;

private:
    QString createEmptyEmbedding(QString name, QString dataType, QString sourceName);

private:
    HsneHierarchy _hierarchy;

    TsneAnalysis _tsne;

    QString _embeddingName;

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
