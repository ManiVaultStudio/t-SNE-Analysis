# -----------------------------------------------------------------------------
# T-SNE Plugin Target
# -----------------------------------------------------------------------------
set(TSNE_PLUGIN "TsneAnalysisPlugin")

# -----------------------------------------------------------------------------
# Source files
# -----------------------------------------------------------------------------
add_subdirectory(tSNE/src)

source_group(Common\\Actions FILES ${TSNE_ACTIONS_SOURCES})
source_group(Tsne FILES ${TSNE_PLUGIN_SOURCES})
source_group(Resources FILES ${TSNE_RESOURCES})

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# -----------------------------------------------------------------------------
# CMake Target
# -----------------------------------------------------------------------------
add_library(${TSNE_PLUGIN} SHARED
    ${DIMENSION_SELECTION_SOURCES}
    ${TSNE_ACTIONS_SOURCES}
    ${TSNE_COMMON_SOURCES}
    ${TSNE_PLUGIN_SOURCES}
	${TSNE_RESOURCES}
)

# -----------------------------------------------------------------------------
# Target include directories
# -----------------------------------------------------------------------------
# For inclusion of Qt generated ui_DimensionSelectionWidget.h
target_include_directories(${TSNE_PLUGIN} PRIVATE ${PROJECT_BINARY_DIR})

# Include HDPS core headers
target_include_directories(${TSNE_PLUGIN} PRIVATE "${MV_INSTALL_DIR}/$<CONFIGURATION>/include/")

target_include_directories(${TSNE_PLUGIN} PRIVATE "Common")

set_HDILib_project_includes(${TSNE_PLUGIN})
set_flann_project_includes(${TSNE_PLUGIN})
set_lz4_project_includes(${TSNE_PLUGIN})


# -----------------------------------------------------------------------------
# Target properties
# -----------------------------------------------------------------------------
# Request C++17, in order to use std::for_each_n with std::execution::par_unseq.
set_target_properties(${TSNE_PLUGIN} PROPERTIES CXX_STANDARD 17)

target_compile_definitions(${TSNE_PLUGIN} PRIVATE QT_MESSAGELOGCONTEXT)

# -----------------------------------------------------------------------------
# Target library linking
# -----------------------------------------------------------------------------
target_link_libraries(${TSNE_PLUGIN} Qt6::Widgets)
target_link_libraries(${TSNE_PLUGIN} Qt6::WebEngineWidgets)

set(HDPS_LINK_PATH "${MV_INSTALL_DIR}/$<CONFIGURATION>/lib")
set(PLUGIN_LINK_PATH "${MV_INSTALL_DIR}/$<CONFIGURATION>/$<IF:$<CXX_COMPILER_ID:MSVC>,lib,Plugins>")
set(HDPS_LINK_SUFFIX $<IF:$<CXX_COMPILER_ID:MSVC>,${CMAKE_LINK_LIBRARY_SUFFIX},${CMAKE_SHARED_LIBRARY_SUFFIX}>)

set(HDPS_LINK_LIBRARY "${HDPS_LINK_PATH}/${CMAKE_SHARED_LIBRARY_PREFIX}HDPS_Public${HDPS_LINK_SUFFIX}")
set(POINTDATA_LINK_LIBRARY "${PLUGIN_LINK_PATH}/${CMAKE_SHARED_LIBRARY_PREFIX}PointData${HDPS_LINK_SUFFIX}") 

target_link_libraries(${TSNE_PLUGIN} "${HDPS_LINK_LIBRARY}")
target_link_libraries(${TSNE_PLUGIN} "${POINTDATA_LINK_LIBRARY}")

target_link_libraries(${TSNE_PLUGIN} ${OPENGL_LIBRARIES})

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(${TSNE_PLUGIN} OpenMP::OpenMP_CXX)
endif()

set_lz4_project_link_libraries(${TSNE_PLUGIN})
set_flann_project_link_libraries(${TSNE_PLUGIN})
set_omp_project_link_libraries(${TSNE_PLUGIN})
set_HDILib_project_link_libraries(${TSNE_PLUGIN})

if(UNIX)
    message(STATUS "pThreads for Linux")
    find_package(Threads REQUIRED)
endif(UNIX)

# -----------------------------------------------------------------------------
# Target installation
# -----------------------------------------------------------------------------
install(TARGETS ${TSNE_PLUGIN}
    RUNTIME DESTINATION Plugins COMPONENT PLUGIN_TSNE # Windows .dll
    LIBRARY DESTINATION Plugins COMPONENT PLUGIN_TSNE # Linux/Mac .so
)

if (NOT DEFINED ENV{CI})
    add_custom_command(TARGET ${TSNE_PLUGIN} POST_BUILD
        COMMAND "${CMAKE_COMMAND}"
        --install ${PROJECT_BINARY_DIR}
        --config $<CONFIGURATION>
        --component PLUGIN_TSNE
        --prefix ${MV_INSTALL_DIR}/$<CONFIGURATION>
        --verbose
    )
endif()

