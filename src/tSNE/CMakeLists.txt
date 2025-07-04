# -----------------------------------------------------------------------------
# T-SNE Plugin Target
# -----------------------------------------------------------------------------
set(TSNE_PLUGIN "TsneAnalysisPlugin")

# -----------------------------------------------------------------------------
# Source files
# -----------------------------------------------------------------------------

set(TSNE_PLUGIN_SOURCES
    TsneAnalysisPlugin.h
    TsneAnalysisPlugin.cpp
)

set(TSNE_ACTIONS_SOURCES
    TsneSettingsAction.h
    TsneSettingsAction.cpp
    GeneralTsneSettingsAction.h
    GeneralTsneSettingsAction.cpp
    InitTsneSettings.h
    InitTsneSettings.cpp
)

set(AUX
    PluginInfo.json
)

source_group(Common FILES ${COMMON_TSNE_SOURCES})
source_group(Common//Actions FILES ${COMMON_ACTIONS_SOURCES})
source_group(Plugin FILES ${TSNE_PLUGIN_SOURCES})
source_group(Actions FILES ${TSNE_ACTIONS_SOURCES})
source_group(Aux FILES ${AUX})

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
target_include_directories(${TSNE_PLUGIN} PRIVATE "${ManiVault_INCLUDE_DIR}")
target_include_directories(${TSNE_PLUGIN} PRIVATE "${COMMON_TSNE_DIR}")

set_HDILib_project_includes(${TSNE_PLUGIN})

# -----------------------------------------------------------------------------
# Target properties
# -----------------------------------------------------------------------------
target_compile_features(${TSNE_PLUGIN} PRIVATE cxx_std_20)

target_compile_definitions(${TSNE_PLUGIN} PRIVATE QT_MESSAGELOGCONTEXT)

if(MV_UNITY_BUILD)
    set_target_properties(${TSNE_PLUGIN} PROPERTIES UNITY_BUILD ON)
endif()

# -----------------------------------------------------------------------------
# Target library linking
# -----------------------------------------------------------------------------
target_link_libraries(${TSNE_PLUGIN} PRIVATE Qt6::Widgets)
target_link_libraries(${TSNE_PLUGIN} PRIVATE Qt6::WebEngineWidgets)

target_link_libraries(${TSNE_PLUGIN} PRIVATE ManiVault::Core)
target_link_libraries(${TSNE_PLUGIN} PRIVATE ManiVault::PointData)

target_link_libraries(${TSNE_PLUGIN} PRIVATE ${OPENGL_LIBRARIES})

if(OpenMP_CXX_FOUND)
    target_link_libraries(${TSNE_PLUGIN} PRIVATE OpenMP::OpenMP_CXX)
endif()

set_flann_project_link_libraries(${TSNE_PLUGIN})
set_HDILib_project_link_libraries(${TSNE_PLUGIN})
set_lz4_project_link_libraries(${TSNE_PLUGIN})

set_optimization_level(${TSNE_PLUGIN} ${MV_SNE_OPTIMIZATION_LEVEL})
mv_check_and_set_AVX(${TSNE_PLUGIN} ${MV_SNE_USE_AVX})

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
        --prefix ${ManiVault_INSTALL_DIR}/$<CONFIGURATION>
        --verbose
    )
endif()

mv_handle_plugin_config(${TSNE_PLUGIN})

# -----------------------------------------------------------------------------
# Misc
# -----------------------------------------------------------------------------
# Automatically set the debug environment (command + working directory) for MSVC
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set_property(TARGET ${TSNE_PLUGIN} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY $<IF:$<CONFIG:DEBUG>,${ManiVault_INSTALL_DIR}/Debug,$<IF:$<CONFIG:RELWITHDEBINFO>,${ManiVault_INSTALL_DIR}/RelWithDebInfo,${ManiVault_INSTALL_DIR}/Release>>)
    set_property(TARGET ${TSNE_PLUGIN} PROPERTY VS_DEBUGGER_COMMAND $<IF:$<CONFIG:DEBUG>,"${ManiVault_INSTALL_DIR}/Debug/ManiVault Studio.exe",$<IF:$<CONFIG:RELWITHDEBINFO>,"${ManiVault_INSTALL_DIR}/RelWithDebInfo/ManiVault Studio.exe","${ManiVault_INSTALL_DIR}/Release/ManiVault Studio.exe">>)
endif()
