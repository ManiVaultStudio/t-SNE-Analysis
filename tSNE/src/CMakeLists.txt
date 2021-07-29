set(DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(TSNE_PLUGIN_SOURCES
    ${DIR}/TsneAnalysisPlugin.h
    ${DIR}/TsneAnalysisPlugin.cpp
    ${DIR}/TsneAnalysisPlugin.json
	${DIR}/DimensionsSettingsAction.h
    ${DIR}/DimensionsSettingsAction.cpp
	${DIR}/GeneralSettingsAction.h
    ${DIR}/GeneralSettingsAction.cpp
	${DIR}/AdvancedSettingsAction.h
    ${DIR}/AdvancedSettingsAction.cpp
    PARENT_SCOPE
)
