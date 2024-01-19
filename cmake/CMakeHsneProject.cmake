# -----------------------------------------------------------------------------
# HSNE Plugin Target
# -----------------------------------------------------------------------------
set(HSNE_PLUGIN "HsneAnalysisPlugin")

# -----------------------------------------------------------------------------
# Source files
# -----------------------------------------------------------------------------
add_subdirectory(HSNE/src)

source_group(Common FILES ${COMMON_TSNE_SOURCES})
source_group(Common//Actions FILES ${COMMON_ACTIONS_SOURCES})
source_group(Actions FILES ${HSNE_ACTIONS_SOURCES})
source_group(Hsne FILES ${HSNE_PLUGIN_SOURCES})
source_group(Resources FILES ${HSNE_RESOURCES})
source_group(Utils FILES ${THIRD_PARTY_JSON})

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# -----------------------------------------------------------------------------
# CMake Target
# -----------------------------------------------------------------------------
add_library(${HSNE_PLUGIN} SHARED
    ${DIMENSION_SELECTION_SOURCES}
    ${HSNE_ACTIONS_SOURCES}
    ${COMMON_TSNE_SOURCES}
    ${COMMON_ACTIONS_SOURCES}
    ${HSNE_PLUGIN_SOURCES}
    ${HSNE_RESOURCES}
    ${THIRD_PARTY_JSON}
)

# -----------------------------------------------------------------------------
# Target include directories
# -----------------------------------------------------------------------------
# For inclusion of Qt generated ui_DimensionSelectionWidget.h
target_include_directories(${HSNE_PLUGIN} PRIVATE ${PROJECT_BINARY_DIR})

# Include HDPS core headers
target_include_directories(${HSNE_PLUGIN} PRIVATE "${MV_INSTALL_DIR}/$<CONFIGURATION>/include/")

target_include_directories(${HSNE_PLUGIN} PRIVATE "Common")
target_include_directories(${HSNE_PLUGIN} PRIVATE "third_party/json")

set_HDILib_project_includes(${HSNE_PLUGIN})
set_flann_project_includes(${HSNE_PLUGIN})
set_lz4_project_includes(${HSNE_PLUGIN})


# -----------------------------------------------------------------------------
# Target properties
# -----------------------------------------------------------------------------
# Request C++17, in order to use std::for_each_n with std::execution::par_unseq.
set_target_properties(${HSNE_PLUGIN} PROPERTIES CXX_STANDARD 17)

target_compile_definitions(${HSNE_PLUGIN} PRIVATE QT_MESSAGELOGCONTEXT)

# -----------------------------------------------------------------------------
# Target library linking
# -----------------------------------------------------------------------------
target_link_libraries(${HSNE_PLUGIN} Qt6::Widgets)
target_link_libraries(${HSNE_PLUGIN} Qt6::WebEngineWidgets)

set(MV_LINK_PATH "${MV_INSTALL_DIR}/$<CONFIGURATION>/lib")
set(PLUGIN_LINK_PATH "${MV_INSTALL_DIR}/$<CONFIGURATION>/$<IF:$<CXX_COMPILER_ID:MSVC>,lib,Plugins>")
set(MV_LINK_SUFFIX $<IF:$<CXX_COMPILER_ID:MSVC>,${CMAKE_LINK_LIBRARY_SUFFIX},${CMAKE_SHARED_LIBRARY_SUFFIX}>)

set(MV_LINK_LIBRARY "${MV_LINK_PATH}/${CMAKE_SHARED_LIBRARY_PREFIX}MV_Public${MV_LINK_SUFFIX}")
set(POINTDATA_LINK_LIBRARY "${PLUGIN_LINK_PATH}/${CMAKE_SHARED_LIBRARY_PREFIX}PointData${MV_LINK_SUFFIX}") 
set(IMAGEDATA_LINK_LIBRARY "${PLUGIN_LINK_PATH}/${CMAKE_SHARED_LIBRARY_PREFIX}ImageData${MV_LINK_SUFFIX}") 

target_link_libraries(${HSNE_PLUGIN} "${MV_LINK_LIBRARY}")
target_link_libraries(${HSNE_PLUGIN} "${POINTDATA_LINK_LIBRARY}")
target_link_libraries(${HSNE_PLUGIN} "${IMAGEDATA_LINK_LIBRARY}")

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(${HSNE_PLUGIN} OpenMP::OpenMP_CXX)
endif()

target_link_libraries(${HSNE_PLUGIN} ${OPENGL_LIBRARIES})

set_flann_project_link_libraries(${HSNE_PLUGIN})
set_HDILib_project_link_libraries(${HSNE_PLUGIN})
set_lz4_project_link_libraries(${HSNE_PLUGIN}) 

if(UNIX)
    message(STATUS "pThreads for Linux")
    find_package(Threads REQUIRED)
endif(UNIX)

# -----------------------------------------------------------------------------
# Target installation
# -----------------------------------------------------------------------------
install(TARGETS ${HSNE_PLUGIN}
    RUNTIME DESTINATION Plugins COMPONENT PLUGIN_HSNE # Windows .dll
    LIBRARY DESTINATION Plugins COMPONENT PLUGIN_HSNE # Linux/Mac .so
)

if (NOT DEFINED ENV{CI})
    add_custom_command(TARGET ${HSNE_PLUGIN} POST_BUILD
        COMMAND "${CMAKE_COMMAND}"
        --install ${PROJECT_BINARY_DIR}
        --config $<CONFIGURATION>
        --component PLUGIN_HSNE
        --prefix ${MV_INSTALL_DIR}/$<CONFIGURATION>
        --verbose
    )
endif()
