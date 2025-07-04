set(COMMON_TSNE_DIR 
    ${CMAKE_CURRENT_SOURCE_DIR}
    CACHE INTERNAL "Common include and source directory"
)

set(COMMON_TSNE_SOURCES
    ${COMMON_TSNE_DIR}/TsneAnalysis.h
    ${COMMON_TSNE_DIR}/TsneAnalysis.cpp
    ${COMMON_TSNE_DIR}/TsneData.h
    ${COMMON_TSNE_DIR}/TsneParameters.h
    ${COMMON_TSNE_DIR}/KnnParameters.h
    ${COMMON_TSNE_DIR}/OffscreenBuffer.h
    ${COMMON_TSNE_DIR}/OffscreenBuffer.cpp
    CACHE INTERNAL "Common tsne sources"
)

set(COMMON_ACTIONS_SOURCES
    ${COMMON_TSNE_DIR}/TsneComputationAction.h
    ${COMMON_TSNE_DIR}/TsneComputationAction.cpp
    ${COMMON_TSNE_DIR}/GradientDescentSettingsAction.h
    ${COMMON_TSNE_DIR}/GradientDescentSettingsAction.cpp
    ${COMMON_TSNE_DIR}/KnnSettingsAction.h
    ${COMMON_TSNE_DIR}/KnnSettingsAction.cpp
    CACHE INTERNAL "Common action sources"
)
