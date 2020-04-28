set(DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(TSNE_PLUGIN_SOURCES
    ${DIR}/TsneAnalysisPlugin.h
    ${DIR}/TsneAnalysisPlugin.cpp
    ${DIR}/TsneAnalysisPlugin.json
    ${DIR}/TsneAnalysis.h
    ${DIR}/TsneAnalysis.cpp
    ${DIR}/TsneSettingsWidget.h
    ${DIR}/TsneSettingsWidget.cpp
    ${DIR}/TsneData.h
    PARENT_SCOPE
)
