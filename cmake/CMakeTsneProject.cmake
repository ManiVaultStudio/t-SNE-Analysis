# -----------------------------------------------------------------------------
# T-SNE Plugin Target
# -----------------------------------------------------------------------------
set(TSNE_PLUGIN "TsneAnalysisPlugin")

# -----------------------------------------------------------------------------
# Source files
# -----------------------------------------------------------------------------
add_subdirectory(src/tSNE)

source_group(Common FILES ${COMMON_TSNE_SOURCES})
source_group(Common//Actions FILES ${COMMON_ACTIONS_SOURCES})
source_group(Tsne FILES ${TSNE_PLUGIN_SOURCES})
source_group(Actions FILES ${TSNE_ACTIONS_SOURCES})

# -----------------------------------------------------------------------------
# CMake Target
# -----------------------------------------------------------------------------
add_library(${TSNE_PLUGIN} SHARED
    ${DIMENSION_SELECTION_SOURCES}
    ${COMMON_TSNE_SOURCES}
    ${COMMON_ACTIONS_SOURCES}
    ${TSNE_PLUGIN_SOURCES}
    ${TSNE_ACTIONS_SOURCES}
)

# -----------------------------------------------------------------------------
# Target include directories
# -----------------------------------------------------------------------------
# Include ManiVault core headers
target_include_directories(${TSNE_PLUGIN} PRIVATE "${MV_INSTALL_DIR}/$<CONFIGURATION>/include/")

target_include_directories(${TSNE_PLUGIN} PRIVATE "src/Common")

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
target_link_libraries(${TSNE_PLUGIN} PRIVATE Qt6::Widgets)
target_link_libraries(${TSNE_PLUGIN} PRIVATE Qt6::WebEngineWidgets)

set(MV_LINK_PATH "${MV_INSTALL_DIR}/$<CONFIGURATION>/lib")
set(PLUGIN_LINK_PATH "${MV_INSTALL_DIR}/$<CONFIGURATION>/$<IF:$<CXX_COMPILER_ID:MSVC>,lib,Plugins>")
set(MV_LINK_SUFFIX $<IF:$<CXX_COMPILER_ID:MSVC>,${CMAKE_LINK_LIBRARY_SUFFIX},${CMAKE_SHARED_LIBRARY_SUFFIX}>)

set(MV_LINK_LIBRARY "${MV_LINK_PATH}/${CMAKE_SHARED_LIBRARY_PREFIX}MV_Public${MV_LINK_SUFFIX}")
set(POINTDATA_LINK_LIBRARY "${PLUGIN_LINK_PATH}/${CMAKE_SHARED_LIBRARY_PREFIX}PointData${MV_LINK_SUFFIX}") 

target_link_libraries(${TSNE_PLUGIN} PRIVATE "${MV_LINK_LIBRARY}")
target_link_libraries(${TSNE_PLUGIN} PRIVATE "${POINTDATA_LINK_LIBRARY}")

target_link_libraries(${TSNE_PLUGIN} PRIVATE ${OPENGL_LIBRARIES})

if(OpenMP_CXX_FOUND)
    target_link_libraries(${TSNE_PLUGIN} PRIVATE OpenMP::OpenMP_CXX)
endif()

set_flann_project_link_libraries(${TSNE_PLUGIN})
set_HDILib_project_link_libraries(${TSNE_PLUGIN})
set_lz4_project_link_libraries(${TSNE_PLUGIN})

set_optimization_level(${TSNE_PLUGIN} ${OPTIMIZATION_LEVEL})
check_and_set_AVX(${TSNE_PLUGIN} ${ENABLE_AVX})

silence_opengl_deprecation(${TSNE_PLUGIN})

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

# -----------------------------------------------------------------------------
# Misc
# -----------------------------------------------------------------------------
# Automatically set the debug environment (command + working directory) for MSVC
if(MSVC)
    set_property(TARGET ${TSNE_PLUGIN} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY $<IF:$<CONFIG:DEBUG>,${MV_INSTALL_DIR}/debug,${MV_INSTALL_DIR}/release>)
    set_property(TARGET ${TSNE_PLUGIN} PROPERTY VS_DEBUGGER_COMMAND $<IF:$<CONFIG:DEBUG>,"${MV_INSTALL_DIR}/debug/ManiVault Studio.exe","${MV_INSTALL_DIR}/release/ManiVault Studio.exe">)
endif()
