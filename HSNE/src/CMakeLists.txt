set(DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(HSNE_PLUGIN_SOURCES
    ${DIR}/HsneAnalysisPlugin.h
    ${DIR}/HsneAnalysisPlugin.cpp
    ${DIR}/HsneAnalysisPlugin.json
    ${DIR}/HsneHierarchy.h
    ${DIR}/HsneHierarchy.cpp
    ${DIR}/HsneParameters.h
    ${DIR}/HsneParameters.cpp
    PARENT_SCOPE
)

set(HSNE_ACTIONS_SOURCES
    ${DIR}/HsneSettingsAction.h
    ${DIR}/HsneSettingsAction.cpp
    ${DIR}/GeneralHsneSettingsAction.h
    ${DIR}/GeneralHsneSettingsAction.cpp
    ${DIR}/AdvancedHsneSettingsAction.h
    ${DIR}/AdvancedHsneSettingsAction.cpp
	${DIR}/HsneScaleAction.h
    ${DIR}/HsneScaleAction.cpp
	PARENT_SCOPE
)

set(HSNE_RESOURCES
    ${DIR}/../res/resources.qrc
    PARENT_SCOPE
)