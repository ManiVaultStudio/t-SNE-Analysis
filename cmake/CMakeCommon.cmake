set(DIR ${CMAKE_CURRENT_SOURCE_DIR}/Common)

set(TSNE_COMMON_SOURCES
    ${DIR}/TsneAnalysis.h
    ${DIR}/TsneAnalysis.cpp
    ${DIR}/TsneData.h
)

set(DIMENSION_SELECTION_SOURCES
    ${DIR}/ModelResetter.h
    ${DIR}/ModelResetter.cpp
    ${DIR}/DimensionSelectionHolder.h
    ${DIR}/DimensionSelectionHolder.cpp
    ${DIR}/DimensionSelectionItemModel.h
    ${DIR}/DimensionSelectionItemModel.cpp
    ${DIR}/DimensionSelectionProxyModel.h
    ${DIR}/DimensionSelectionProxyModel.cpp
    ${DIR}/DimensionSelectionWidget.h
    ${DIR}/DimensionSelectionWidget.cpp
    #${DIR}/GeneralSettingsWidget.h
    #${DIR}/GeneralSettingsWidget.cpp
)

set(UI_FILES
    ${DIR}/ui/DimensionSelectionWidget.ui
)

# The following 4 macros are designed to support the use of user supplied
# libraries or the PREBUILT libraries for flann and HDILib from the LKEB
# artifactory.

# TODO complete for MacOS and Linux

# Include flann includes via target_include_directories to the current project
macro(set_flann_project_includes)
    if(USE_PREBUILT_LIBS)
        MESSAGE( STATUS "Including PREBUILT flann ${CMAKE_SOURCE_DIR}/flann")
        target_include_directories("${CMAKE_PROJECT_NAME}" PRIVATE "${CMAKE_SOURCE_DIR}/flann/include")
    else()
        if(MSVC)
            target_include_directories ("${CMAKE_PROJECT_NAME}" PRIVATE "${CMAKE_SOURCE_DIR}/tSNE/lib/Flann/Win/include")
        endif()

        if(APPLE)
            target_include_directories ("${CMAKE_PROJECT_NAME}" PRIVATE "${CMAKE_SOURCE_DIR}/tSNE/lib/Flann/OSX/include")
        endif()

        if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
            target_include_directories ("${CMAKE_PROJECT_NAME}" PRIVATE "${CMAKE_SOURCE_DIR}/tSNE/lib/Flann/Linux/include")
        endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    endif()
endmacro()

# Include HDILib includes via target_include_directories to the current project
macro(set_HDILib_project_includes)
    if(USE_PREBUILT_LIBS)
        MESSAGE( STATUS "Including PREBUILT HDILib ${HDILib_INCLUDE_DIR}")
        target_include_directories("${CMAKE_PROJECT_NAME}" PRIVATE "${HDILib_INCLUDE_DIR}")
    else()
        target_include_directories("${CMAKE_PROJECT_NAME}" PRIVATE "${CMAKE_SOURCE_DIR}/tSNE/lib/HDI/include")
    endif()
endmacro()

# Link the flann libraries via target_link_library to the current project
macro(set_flann_project_link_libraries)
    if(USE_PREBUILT_LIBS)
        MESSAGE( STATUS "Linking PREBUILT flann libraries...")
        if(MSVC)
            target_link_libraries("${CMAKE_PROJECT_NAME}" debug "${CMAKE_SOURCE_DIR}/flann/lib/Debug/flann.lib")
            target_link_libraries("${CMAKE_PROJECT_NAME}" optimized "${CMAKE_SOURCE_DIR}/flann/lib/Release/flann.lib")
        endif()
    else()
        if(MSVC)
            MESSAGE( STATUS "Linking Windows flann libraries...")
            target_link_libraries("${CMAKE_PROJECT_NAME}" debug "${CMAKE_SOURCE_DIR}/tSNE/lib/Flann/Win/Debug/flann_cpp_s.lib")
            target_link_libraries("${CMAKE_PROJECT_NAME}" optimized "${CMAKE_SOURCE_DIR}/tSNE/lib/Flann/Win/Release/flann_cpp_s.lib")
        endif()

        if(APPLE)
            MESSAGE( STATUS "Linking Mac OS X libraries...")
            target_link_libraries("${CMAKE_PROJECT_NAME}" "${CMAKE_SOURCE_DIR}/tSNE/lib/Flann/OSX/libflann_s.a")
        endif()
    endif()
endmacro()

# Link the HDILib libraries via target_link_library to the current project
macro(set_HDILib_project_link_libraries)
    if(USE_PREBUILT_LIBS)
        MESSAGE( STATUS "Linking PREBUILT HDILib libraries...")
        target_link_libraries("${CMAKE_PROJECT_NAME}" ${HDILib_LINK_LIBS})
    else()
        if(MSVC)
            MESSAGE( STATUS "Linking Windows HDILib libraries...")
            target_link_libraries("${CMAKE_PROJECT_NAME}" debug "${CMAKE_SOURCE_DIR}/tSNE/lib/HDI/Win/Debug/hdidimensionalityreduction.lib")
            target_link_libraries("${CMAKE_PROJECT_NAME}" debug "${CMAKE_SOURCE_DIR}/tSNE/lib/HDI/Win/Debug/hdidata.lib")
            target_link_libraries("${CMAKE_PROJECT_NAME}" debug "${CMAKE_SOURCE_DIR}/tSNE/lib/HDI/Win/Debug/hdiutils.lib")
            target_link_libraries("${CMAKE_PROJECT_NAME}" optimized "${CMAKE_SOURCE_DIR}/tSNE/lib/HDI/Win/Release/hdidimensionalityreduction.lib")
            target_link_libraries("${CMAKE_PROJECT_NAME}" optimized "${CMAKE_SOURCE_DIR}/tSNE/lib/HDI/Win/Release/hdidata.lib")
            target_link_libraries("${CMAKE_PROJECT_NAME}" optimized "${CMAKE_SOURCE_DIR}/tSNE/lib/HDI/Win/Release/hdiutils.lib")
            target_link_libraries("${CMAKE_PROJECT_NAME}" debug "${CMAKE_SOURCE_DIR}/tSNE/lib/Flann/Win/Debug/flann_cpp_s.lib")
            target_link_libraries("${CMAKE_PROJECT_NAME}" optimized "${CMAKE_SOURCE_DIR}/tSNE/lib/Flann/Win/Release/flann_cpp_s.lib")
        endif()
        if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
            MESSAGE( STATUS "Linking Linux libraries...")
            target_link_libraries("${CMAKE_PROJECT_NAME}" "${CMAKE_SOURCE_DIR}/tSNE/lib/AtSNE/Linux/libbh_t_sne_library.a")
        endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        if(APPLE)
            MESSAGE( STATUS "Linking Mac OS X HDILib libraries...")
            target_link_libraries("${CMAKE_PROJECT_NAME}" debug "${CMAKE_SOURCE_DIR}/tSNE/lib/AtSNE/OSX/Debug/libbh_t_sne_library.a")
            target_link_libraries("${CMAKE_PROJECT_NAME}" optimized "${CMAKE_SOURCE_DIR}/tSNE/lib/AtSNE/OSX/Release/libbh_t_sne_library.a")
        endif()
    endif()
endmacro()
