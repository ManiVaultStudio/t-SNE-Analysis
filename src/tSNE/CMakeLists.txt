set(DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(TSNE_PLUGIN_SOURCES
    ${DIR}/TsneAnalysisPlugin.h
    ${DIR}/TsneAnalysisPlugin.cpp
    ${DIR}/TsneAnalysisPlugin.json
    PARENT_SCOPE
)

set(TSNE_ACTIONS_SOURCES
    ${DIR}/TsneSettingsAction.h
    ${DIR}/TsneSettingsAction.cpp
    ${DIR}/GeneralTsneSettingsAction.h
    ${DIR}/GeneralTsneSettingsAction.cpp
    ${DIR}/InitTsneSettings.h
    ${DIR}/InitTsneSettings.cpp
    PARENT_SCOPE
)
