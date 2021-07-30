set(DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(TSNE_PLUGIN_SOURCES
    ${DIR}/TsneAnalysisPlugin.h
    ${DIR}/TsneAnalysisPlugin.cpp
    ${DIR}/TsneAnalysisPlugin.json
	${DIR}/DimensionSelectionAction.h
    ${DIR}/DimensionSelectionAction.cpp
	${DIR}/TsneSettingsAction.h
    ${DIR}/TsneSettingsAction.cpp
	${DIR}/GeneralSettingsAction.h
    ${DIR}/GeneralSettingsAction.cpp
	${DIR}/AdvancedSettingsAction.h
    ${DIR}/AdvancedSettingsAction.cpp
    PARENT_SCOPE
)
