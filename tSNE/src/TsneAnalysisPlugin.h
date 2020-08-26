#pragma once

#include <AnalysisPlugin.h>

#include "TsneAnalysis.h"

class TsneSettingsWidget;

using namespace hdps::plugin;
using namespace hdps::gui;

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

    SettingsWidget* const getSettings() override;

    void startComputation();
    void stopComputation();

public: // GUI

	/**
	 * Generates a context menu for display in other (view) plugins
	 * @param kind Kind of plugin in which the context menu will be shown
	 * @return Context menu
	 */
	QMenu* contextMenu(const QVariant& context) override;

public slots:
    void dataSetPicked(const QString& name);
    void onKnnAlgorithmPicked(const int index);
    void onDistanceMetricPicked(const int index);
    void onNewEmbedding();

private:
    void initializeTsne();

    TsneAnalysis _tsne;

    std::unique_ptr<TsneSettingsWidget> _settings;
    QString _embeddingName;
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
