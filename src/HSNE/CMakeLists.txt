# -----------------------------------------------------------------------------
# HSNE Plugin Target
# -----------------------------------------------------------------------------
set(HSNE_PLUGIN "HsneAnalysisPlugin")

# -----------------------------------------------------------------------------
# Source files
# -----------------------------------------------------------------------------

set(HSNE_PLUGIN_SOURCES
    HsneAnalysisPlugin.h
    HsneAnalysisPlugin.cpp
    HsneHierarchy.h
    HsneHierarchy.cpp
    HsneParameters.h
    HsneRecomputeWarningDialog.h
    Globals.h
    HsneUtilities.h
    HsneUtilities.cpp
)

set(HSNE_ACTIONS_SOURCES
    HsneSettingsAction.h
    HsneSettingsAction.cpp
    GeneralHsneSettingsAction.h
    GeneralHsneSettingsAction.cpp
    HierarchyConstructionSettingsAction.h
    HierarchyConstructionSettingsAction.cpp
    HsneScaleAction.h
    HsneScaleAction.cpp
)

set(AUX
    PluginInfo.json
)

source_group(Common FILES ${COMMON_TSNE_SOURCES})
source_group(Common//Actions FILES ${COMMON_ACTIONS_SOURCES})
source_group(Actions FILES ${HSNE_ACTIONS_SOURCES})
source_group(Plugin FILES ${HSNE_PLUGIN_SOURCES})
source_group(Utils FILES ${THIRD_PARTY_JSON})
source_group(Aux FILES ${AUX})

# -----------------------------------------------------------------------------
# CMake Target
# -----------------------------------------------------------------------------
add_library(${HSNE_PLUGIN} SHARED
    ${DIMENSION_SELECTION_SOURCES}
    ${HSNE_ACTIONS_SOURCES}
    ${COMMON_TSNE_SOURCES}
    ${COMMON_ACTIONS_SOURCES}
    ${HSNE_PLUGIN_SOURCES}
    ${THIRD_PARTY_JSON}
)

# -----------------------------------------------------------------------------
# Target include directories
# -----------------------------------------------------------------------------
target_include_directories(${HSNE_PLUGIN} PRIVATE "${ManiVault_INCLUDE_DIR}")
target_include_directories(${HSNE_PLUGIN} PRIVATE "${COMMON_TSNE_DIR}")
target_include_directories(${HSNE_PLUGIN} PRIVATE "${THIRDPARTY_INCLUDE_DIR}")

set_HDILib_project_includes(${HSNE_PLUGIN})

# -----------------------------------------------------------------------------
# Target properties
# -----------------------------------------------------------------------------
target_compile_features(${HSNE_PLUGIN} PRIVATE cxx_std_20)

target_compile_definitions(${HSNE_PLUGIN} PRIVATE QT_MESSAGELOGCONTEXT)

if(MV_UNITY_BUILD)
    set_target_properties(${HSNE_PLUGIN} PROPERTIES UNITY_BUILD ON)
endif()

# -----------------------------------------------------------------------------
# Target library linking
# -----------------------------------------------------------------------------
target_link_libraries(${HSNE_PLUGIN} PRIVATE Qt6::Widgets)
target_link_libraries(${HSNE_PLUGIN} PRIVATE Qt6::WebEngineWidgets)

target_link_libraries(${HSNE_PLUGIN} PRIVATE ManiVault::Core)
target_link_libraries(${HSNE_PLUGIN} PRIVATE ManiVault::PointData)
target_link_libraries(${HSNE_PLUGIN} PRIVATE ManiVault::ImageData)

target_link_libraries(${HSNE_PLUGIN} PRIVATE ${OPENGL_LIBRARIES})

if(OpenMP_CXX_FOUND)
    target_link_libraries(${HSNE_PLUGIN} PRIVATE OpenMP::OpenMP_CXX)
endif()

set_flann_project_link_libraries(${HSNE_PLUGIN})
set_HDILib_project_link_libraries(${HSNE_PLUGIN})
set_lz4_project_link_libraries(${HSNE_PLUGIN}) 

set_optimization_level(${HSNE_PLUGIN} ${MV_SNE_OPTIMIZATION_LEVEL})
mv_check_and_set_AVX(${HSNE_PLUGIN} ${MV_SNE_USE_AVX})

silence_opengl_deprecation(${HSNE_PLUGIN})

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
        --prefix ${ManiVault_INSTALL_DIR}/$<CONFIGURATION>
        --verbose
    )
endif()

mv_handle_plugin_config(${HSNE_PLUGIN})

# -----------------------------------------------------------------------------
# Misc
# -----------------------------------------------------------------------------
# Automatically set the debug environment (command + working directory) for MSVC
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set_property(TARGET ${HSNE_PLUGIN} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY $<IF:$<CONFIG:DEBUG>,${ManiVault_INSTALL_DIR}/Debug,$<IF:$<CONFIG:RELWITHDEBINFO>,${ManiVault_INSTALL_DIR}/RelWithDebInfo,${ManiVault_INSTALL_DIR}/Release>>)
    set_property(TARGET ${HSNE_PLUGIN} PROPERTY VS_DEBUGGER_COMMAND $<IF:$<CONFIG:DEBUG>,"${ManiVault_INSTALL_DIR}/Debug/ManiVault Studio.exe",$<IF:$<CONFIG:RELWITHDEBINFO>,"${ManiVault_INSTALL_DIR}/RelWithDebInfo/ManiVault Studio.exe","${ManiVault_INSTALL_DIR}/Release/ManiVault Studio.exe">>)
endif()
