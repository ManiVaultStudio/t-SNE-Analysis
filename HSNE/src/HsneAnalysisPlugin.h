#pragma once

#include <AnalysisPlugin.h>

#include "HsneHierarchy.h"
#include "TsneAnalysis.h"
#include "HsneSettingsWidget.h"
#include "HsneSettingsAction.h"

using namespace hdps::plugin;
using namespace hdps::gui;

// =============================================================================
// View
// =============================================================================

class HsneAnalysisPlugin : public QObject, public AnalysisPlugin
{
    Q_OBJECT
public:
    HsneAnalysisPlugin();
    ~HsneAnalysisPlugin(void) override;

    void init() override;

    /**
     * This function is called from the core system whenever data is added.
     *
     * @param name The name of the added dataset
     */
    void onDataEvent(hdps::DataEvent* dataEvent);

    void computeTopLevelEmbedding();
    void drillIn(QString embeddingName);

public slots:
    void dataSetPicked(const QString& name);
    void startComputation();
    void onDrillIn();
    void onNewEmbedding(const TsneData& tsneData);

private:
    HsneHierarchy _hierarchy;

    TsneAnalysis _tsne;

    QString _embeddingNameBase;
    QString _embeddingName;

    //std::unique_ptr<HsneSettingsWidget> _settings;
    HsneSettingsAction      _hsneSettingsAction;        /** Hsne settings action */
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

    hdps::DataTypes supportedDataTypes() const override;
};
