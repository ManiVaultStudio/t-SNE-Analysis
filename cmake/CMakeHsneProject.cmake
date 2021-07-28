# Hsne Plugin
set(HSNE_PLUGIN "HsneAnalysisPlugin")

project(${HSNE_PLUGIN})

add_subdirectory(HSNE/src)
# Normalize the incoming install path
#file(TO_CMAKE_PATH $ENV{HDPS_INSTALL_DIR} INSTALL_DIR)

source_group( DimensionSelection FILES ${DIMENSION_SELECTION_SOURCES})
source_group( Hsne FILES ${HSNE_PLUGIN_SOURCES})
find_package(OpenGL REQUIRED)

QT5_WRAP_UI(UI_HEADERS ${UI_FILES})
if(NOT WIN32)
    find_package(Threads)
endif()

add_library(${HSNE_PLUGIN} SHARED
    ${DIMENSION_SELECTION_SOURCES}
    ${TSNE_COMMON_SOURCES}
    ${HSNE_PLUGIN_SOURCES}
    ${UI_FILES}
)
include_directories("${INSTALL_DIR}/$<CONFIGURATION>/include/")
set_HDILib_project_includes()
include_directories("Common")
set_flann_project_includes()

# Request C++17, in order to use std::for_each_n with std::execution::par_unseq.
set_property(TARGET ${HSNE_PLUGIN} PROPERTY CXX_STANDARD 17)

target_compile_definitions(${HSNE_PLUGIN} PRIVATE QT_MESSAGELOGCONTEXT)

target_link_libraries(${HSNE_PLUGIN} Qt5::Widgets)
target_link_libraries(${HSNE_PLUGIN} Qt5::WebEngineWidgets)
if(MSVC)
	set(LIB_SUFFIX "${CMAKE_STATIC_LIBRARY_SUFFIX}")
else()
	set(LIB_SUFFIX "${CMAKE_SHARED_LIBRARY_SUFFIX}")
endif()
target_link_libraries(${HSNE_PLUGIN} "${INSTALL_DIR}/$<CONFIGURATION>/lib/${CMAKE_SHARED_LIBRARY_PREFIX}HDPS_Public${LIB_SUFFIX}")
target_link_libraries(${HSNE_PLUGIN} "${INSTALL_DIR}/$<CONFIGURATION>/lib/${CMAKE_SHARED_LIBRARY_PREFIX}PointData${LIB_SUFFIX}")
target_link_libraries(${HSNE_PLUGIN} ${OPENGL_LIBRARIES})
set_flann_project_link_libraries()
set_HDILib_project_link_libraries()

install(TARGETS ${HSNE_PLUGIN}
   RUNTIME DESTINATION "${INSTALL_DIR}/$<CONFIGURATION>/Plugins" COMPONENT HSNE_SHAREDLIB
)

if (NOT DEFINED ENV{CI})
    add_custom_command(TARGET ${HSNE_PLUGIN} POST_BUILD
        COMMAND "${CMAKE_COMMAND}"
        --install ${CMAKE_BINARY_DIR}
        --config $<CONFIGURATION>
        --component HSNE_SHAREDLIB
        --prefix ${INSTALL_DIR}/$<CONFIGURATION>
    )
endif()
