set(DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(COMMON_TSNE_SOURCES
    ${DIR}/TsneAnalysis.h
    ${DIR}/TsneAnalysis.cpp
    ${DIR}/TsneData.h
    ${DIR}/TsneParameters.h
    ${DIR}/KnnParameters.h
    ${DIR}/OffscreenBuffer.h
    ${DIR}/OffscreenBuffer.cpp
    PARENT_SCOPE
)

set(COMMON_ACTIONS_SOURCES
    ${DIR}/TsneComputationAction.h
    ${DIR}/TsneComputationAction.cpp
    ${DIR}/GradientDescentSettingsAction.h
    ${DIR}/GradientDescentSettingsAction.cpp
    ${DIR}/KnnSettingsAction.h
    ${DIR}/KnnSettingsAction.cpp
    PARENT_SCOPE
)
