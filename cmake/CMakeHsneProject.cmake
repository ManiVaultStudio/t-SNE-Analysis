# -----------------------------------------------------------------------------
# HSNE Plugin Target
# -----------------------------------------------------------------------------
set(HSNE_PLUGIN "HsneAnalysisPlugin")

# -----------------------------------------------------------------------------
# Source files
# -----------------------------------------------------------------------------
add_subdirectory(src/HSNE)

source_group(Common FILES ${COMMON_TSNE_SOURCES})
source_group(Common//Actions FILES ${COMMON_ACTIONS_SOURCES})
source_group(Actions FILES ${HSNE_ACTIONS_SOURCES})
source_group(Hsne FILES ${HSNE_PLUGIN_SOURCES})
source_group(Utils FILES ${THIRD_PARTY_JSON})

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
target_include_directories(${HSNE_PLUGIN} PRIVATE "src/Common")
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

set_optimization_level(${HSNE_PLUGIN} ${OPTIMIZATION_LEVEL})
check_and_set_AVX(${HSNE_PLUGIN} ${ENABLE_AVX})

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

# -----------------------------------------------------------------------------
# Misc
# -----------------------------------------------------------------------------
# Automatically set the debug environment (command + working directory) for MSVC
if(MSVC)
    set_property(TARGET ${HSNE_PLUGIN} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY $<IF:$<CONFIG:DEBUG>,${ManiVault_INSTALL_DIR}/debug,${ManiVault_INSTALL_DIR}/release>)
    set_property(TARGET ${HSNE_PLUGIN} PROPERTY VS_DEBUGGER_COMMAND $<IF:$<CONFIG:DEBUG>,"${ManiVault_INSTALL_DIR}/debug/ManiVault Studio.exe","${ManiVault_INSTALL_DIR}/release/ManiVault Studio.exe">)
endif()
