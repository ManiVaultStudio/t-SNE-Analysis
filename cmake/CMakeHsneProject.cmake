# -----------------------------------------------------------------------------
# HSNE Plugin Target
# -----------------------------------------------------------------------------
set(HSNE_PLUGIN "HsneAnalysisPlugin")

# -----------------------------------------------------------------------------
# Source files
# -----------------------------------------------------------------------------
add_subdirectory(HSNE/src)

source_group(Common//Actions FILES ${TSNE_ACTIONS_SOURCES})
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
    ${TSNE_ACTIONS_SOURCES}
    ${TSNE_COMMON_SOURCES}
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
target_include_directories(${HSNE_PLUGIN} PRIVATE "${INSTALL_DIR}/$<CONFIGURATION>/include/")

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
if(MSVC)
    set(LIB_SUFFIX "${CMAKE_STATIC_LIBRARY_SUFFIX}")
else()
    set(LIB_SUFFIX "${CMAKE_SHARED_LIBRARY_SUFFIX}")
endif()

# Add OpenMP for faster hierarchy initialization
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(${HSNE_PLUGIN} OpenMP::OpenMP_CXX)
endif()

target_link_libraries(${HSNE_PLUGIN} "${INSTALL_DIR}/$<CONFIGURATION>/lib/${CMAKE_SHARED_LIBRARY_PREFIX}HDPS_Public${LIB_SUFFIX}")
target_link_libraries(${HSNE_PLUGIN} "${INSTALL_DIR}/$<CONFIGURATION>/lib/${CMAKE_SHARED_LIBRARY_PREFIX}PointData${LIB_SUFFIX}")
target_link_libraries(${HSNE_PLUGIN} "${INSTALL_DIR}/$<CONFIGURATION>/lib/${CMAKE_SHARED_LIBRARY_PREFIX}ImageData${LIB_SUFFIX}")

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
    RUNTIME DESTINATION Plugins COMPONENT HSNE_SHAREDLIB #.dll
    LIBRARY DESTINATION Plugins COMPONENT LINKLIB #.so .dylib
    ARCHIVE DESTINATION lib COMPONENT LINKLIB       # .lib .a
)

if (NOT DEFINED ENV{CI})
    add_custom_command(TARGET ${HSNE_PLUGIN} POST_BUILD
        COMMAND "${CMAKE_COMMAND}"
        --install ${PROJECT_BINARY_DIR}
        --config $<CONFIGURATION>
        --component HSNE_SHAREDLIB
        --prefix ${INSTALL_DIR}/$<CONFIGURATION>
    )
endif()
