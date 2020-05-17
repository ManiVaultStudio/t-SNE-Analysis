# Hsne Plugin
set(HSNE_PLUGIN "HsneAnalysisPlugin")

project(${HSNE_PLUGIN})

add_subdirectory(HSNE/src)

source_group( DimensionSelection FILES ${DIMENSION_SELECTION_SOURCES})
source_group( Hsne FILES ${HSNE_PLUGIN_SOURCES})

QT5_WRAP_UI(UI_HEADERS ${UI_FILES})

include_directories("$ENV{HDPS_INSTALL_DIR}/$<CONFIGURATION>/include/")
include_directories ("HSNE/lib/HDI/include")

if(MSVC)
    include_directories ("HSNE/lib/Flann/Win/include")
endif(MSVC)

if(APPLE)
    include_directories ("HSNE/lib/Flann/OSX/include")
endif(APPLE)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    include_directories ("HSNE/lib/Flann/Linux/include")
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

add_library(${HSNE_PLUGIN} SHARED
    ${DIMENSION_SELECTION_SOURCES}
    ${HSNE_PLUGIN_SOURCES}
    ${UI_FILES}
)

# Request C++17, in order to use std::for_each_n with std::execution::par_unseq.
set_property(TARGET ${HSNE_PLUGIN} PROPERTY CXX_STANDARD 17)

target_compile_definitions(${HSNE_PLUGIN} PRIVATE QT_MESSAGELOGCONTEXT)

target_link_libraries(${HSNE_PLUGIN} Qt5::Widgets)
target_link_libraries(${HSNE_PLUGIN} Qt5::WebEngineWidgets)
target_link_libraries(${HSNE_PLUGIN} "$ENV{HDPS_INSTALL_DIR}/$<CONFIGURATION>/lib/HDPS_Public.lib")
target_link_libraries(${HSNE_PLUGIN} "$ENV{HDPS_INSTALL_DIR}/$<CONFIGURATION>/lib/PointData.lib")

if(MSVC)
    MESSAGE( STATUS "Linking Windows libraries...")
    target_link_libraries(${HSNE_PLUGIN} debug "${CMAKE_CURRENT_SOURCE_DIR}/HSNE/lib/HDI/Win/Debug/hdidimensionalityreduction.lib")
    target_link_libraries(${HSNE_PLUGIN} debug "${CMAKE_CURRENT_SOURCE_DIR}/HSNE/lib/HDI/Win/Debug/hdidata.lib")
    target_link_libraries(${HSNE_PLUGIN} debug "${CMAKE_CURRENT_SOURCE_DIR}/HSNE/lib/HDI/Win/Debug/hdiutils.lib")
    target_link_libraries(${HSNE_PLUGIN} optimized "${CMAKE_CURRENT_SOURCE_DIR}/HSNE/lib/HDI/Win/Release/hdidimensionalityreduction.lib")
    target_link_libraries(${HSNE_PLUGIN} optimized "${CMAKE_CURRENT_SOURCE_DIR}/HSNE/lib/HDI/Win/Release/hdidata.lib")
    target_link_libraries(${HSNE_PLUGIN} optimized "${CMAKE_CURRENT_SOURCE_DIR}/HSNE/lib/HDI/Win/Release/hdiutils.lib")

    target_link_libraries(${HSNE_PLUGIN} debug "${CMAKE_CURRENT_SOURCE_DIR}/HSNE/lib/Flann/Win/Debug/flann_cpp_s.lib")
    target_link_libraries(${HSNE_PLUGIN} optimized "${CMAKE_CURRENT_SOURCE_DIR}/HSNE/lib/Flann/Win/Release/flann_cpp_s.lib")
endif(MSVC)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    MESSAGE( STATUS "Linking Linux libraries...")
    target_link_libraries(${HSNE_PLUGIN} "${CMAKE_CURRENT_SOURCE_DIR}/HSNE/lib/AtSNE/Linux/libbh_t_sne_library.a")
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

if(APPLE)
    MESSAGE( STATUS "Linking Mac OS X libraries...")

    target_link_libraries(${HSNE_PLUGIN} debug "${CMAKE_CURRENT_SOURCE_DIR}/HSNE/lib/AtSNE/OSX/Debug/libbh_t_sne_library.a")
    target_link_libraries(${HSNE_PLUGIN} optimized "${CMAKE_CURRENT_SOURCE_DIR}/HSNE/lib/AtSNE/OSX/Release/libbh_t_sne_library.a")

    target_link_libraries(${HSNE_PLUGIN} "${CMAKE_CURRENT_SOURCE_DIR}/HSNE/lib/Flann/OSX/libflann_s.a")
endif(APPLE)

add_custom_command(TARGET ${HSNE_PLUGIN} POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "$<TARGET_FILE:${HSNE_PLUGIN}>"
    "$ENV{HDPS_INSTALL_DIR}/$<CONFIGURATION>/Plugins/$<TARGET_FILE_NAME:${HSNE_PLUGIN}>"
)
