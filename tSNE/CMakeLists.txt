# Tsne Plugin
set(TSNE_PLUGIN "TsneAnalysisPlugin")

PROJECT(${TSNE_PLUGIN})

add_subdirectory(src)
# Normalize the incoming install path
file(TO_CMAKE_PATH $ENV{HDPS_INSTALL_DIR} INSTALL_DIR)

source_group( DimensionSelection FILES ${DIMENSION_SELECTION_SOURCES})
source_group( Tsne FILES ${TSNE_PLUGIN_SOURCES})

QT5_WRAP_UI(UI_HEADERS ${UI_FILES})

include_directories("${INSTALL_DIR}/$<CONFIGURATION>/include/")
include_directories ("lib/HDI/include")

if(MSVC)
    include_directories ("lib/Flann/Win/include")
endif(MSVC)

if(APPLE)
    include_directories ("lib/Flann/OSX/include")
endif(APPLE)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    include_directories ("lib/Flann/Linux/include")
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
message(${DIMENSION_SELECTION_SOURCES})
add_library(${TSNE_PLUGIN} SHARED
    ${DIMENSION_SELECTION_SOURCES}
    ${TSNE_PLUGIN_SOURCES}
    ${UI_FILES}
)

# Request C++17, in order to use std::for_each_n with std::execution::par_unseq.
set_property(TARGET ${TSNE_PLUGIN} PROPERTY CXX_STANDARD 17)

target_compile_definitions(${TSNE_PLUGIN} PRIVATE QT_MESSAGELOGCONTEXT)

target_link_libraries(${TSNE_PLUGIN} Qt5::Widgets)
target_link_libraries(${TSNE_PLUGIN} Qt5::WebEngineWidgets)
target_link_libraries(${TSNE_PLUGIN} "${INSTALL_DIR}/$<CONFIGURATION>/lib/HDPS_Public.lib")
target_link_libraries(${TSNE_PLUGIN} "${INSTALL_DIR}/$<CONFIGURATION>/lib/PointData.lib")

if(MSVC)
    MESSAGE( STATUS "Linking Windows libraries...")
    target_link_libraries(${TSNE_PLUGIN} debug "${CMAKE_CURRENT_SOURCE_DIR}/lib/HDI/Win/Debug/hdidimensionalityreduction.lib")
    target_link_libraries(${TSNE_PLUGIN} debug "${CMAKE_CURRENT_SOURCE_DIR}/lib/HDI/Win/Debug/hdidata.lib")
    target_link_libraries(${TSNE_PLUGIN} debug "${CMAKE_CURRENT_SOURCE_DIR}/lib/HDI/Win/Debug/hdiutils.lib")
    target_link_libraries(${TSNE_PLUGIN} optimized "${CMAKE_CURRENT_SOURCE_DIR}/lib/HDI/Win/Release/hdidimensionalityreduction.lib")
    target_link_libraries(${TSNE_PLUGIN} optimized "${CMAKE_CURRENT_SOURCE_DIR}/lib/HDI/Win/Release/hdidata.lib")
    target_link_libraries(${TSNE_PLUGIN} optimized "${CMAKE_CURRENT_SOURCE_DIR}/lib/HDI/Win/Release/hdiutils.lib")

    target_link_libraries(${TSNE_PLUGIN} debug "${CMAKE_CURRENT_SOURCE_DIR}/lib/Flann/Win/Debug/flann_cpp_s.lib")
    target_link_libraries(${TSNE_PLUGIN} optimized "${CMAKE_CURRENT_SOURCE_DIR}/lib/Flann/Win/Release/flann_cpp_s.lib")
endif(MSVC)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    MESSAGE( STATUS "Linking Linux libraries...")
    target_link_libraries(${TSNE_PLUGIN} "${CMAKE_CURRENT_SOURCE_DIR}/lib/AtSNE/Linux/libbh_t_sne_library.a")
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

if(APPLE)
    MESSAGE( STATUS "Linking Mac OS X libraries...")

    target_link_libraries(${TSNE_PLUGIN} debug "${CMAKE_CURRENT_SOURCE_DIR}/lib/AtSNE/OSX/Debug/libbh_t_sne_library.a")
    target_link_libraries(${TSNE_PLUGIN} optimized "${CMAKE_CURRENT_SOURCE_DIR}/lib/AtSNE/OSX/Release/libbh_t_sne_library.a")

    target_link_libraries(${TSNE_PLUGIN} "${CMAKE_CURRENT_SOURCE_DIR}/lib/Flann/OSX/libflann_s.a")
endif(APPLE)

add_custom_command(TARGET ${TSNE_PLUGIN} POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "$<TARGET_FILE:${TSNE_PLUGIN}>"
    "${INSTALL_DIR}/$<CONFIGURATION>/Plugins/$<TARGET_FILE_NAME:${TSNE_PLUGIN}>"
)
