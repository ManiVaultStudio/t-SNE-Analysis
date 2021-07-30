set(DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(HSNE_PLUGIN_SOURCES
    ${DIR}/HsneAnalysisPlugin.h
    ${DIR}/HsneAnalysisPlugin.cpp
    ${DIR}/HsneAnalysisPlugin.json
    ${DIR}/HsneHierarchy.h
    ${DIR}/HsneHierarchy.cpp
    ${DIR}/HsneParameters.h
    ${DIR}/HsneParameters.cpp
    ${DIR}/HsneSettingsWidget.h
    ${DIR}/HsneSettingsWidget.cpp
	${DIR}/HsneSettingsAction.h
    ${DIR}/HsneSettingsAction.cpp
    PARENT_SCOPE
)
