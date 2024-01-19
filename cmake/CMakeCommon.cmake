set(DIR ${CMAKE_CURRENT_SOURCE_DIR}/Common)

set(COMMON_TSNE_SOURCES
    ${DIR}/TsneAnalysis.h
    ${DIR}/TsneAnalysis.cpp
    ${DIR}/TsneData.h
    ${DIR}/TsneParameters.h
    ${DIR}/KnnParameters.h
    ${DIR}/OffscreenBuffer.h
    ${DIR}/OffscreenBuffer.cpp
)

set(COMMON_ACTIONS_SOURCES
    ${DIR}/TsneComputationAction.h
    ${DIR}/TsneComputationAction.cpp
    ${DIR}/GradientDescentSettingsAction.h
    ${DIR}/GradientDescentSettingsAction.cpp
    ${DIR}/KnnSettingsAction.h
    ${DIR}/KnnSettingsAction.cpp
)

set(THIRD_PARTY_JSON
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/json/nlohmann/json.hpp
)

# The following 4 macros are designed to support the use of user supplied
# libraries or the PREBUILT libraries for flann and HDILib from the LKEB
# artifactory.

# TODO complete for MacOS and Linux

# Include flann includes via target_include_directories to the current project
macro(set_flann_project_includes target)
    if(USE_ARTIFACTORY_LIBS)
        MESSAGE( STATUS "Including PREBUILT flann ${LIBRARY_INSTALL_DIR}/flann")
        target_include_directories("${target}" PRIVATE "${LIBRARY_INSTALL_DIR}/flann/include")
    else()
        if(MSVC)
            target_include_directories ("${target}" PRIVATE "${LIBRARY_INSTALL_DIR}/tSNE/lib/Flann/Win/include")
        endif()

        if(APPLE)
            target_include_directories ("${target}" PRIVATE "${LIBRARY_INSTALL_DIR}/tSNE/lib/Flann/OSX/include")
        endif()

        if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
            target_include_directories ("${target}" PRIVATE "${LIBRARY_INSTALL_DIR}/tSNE/lib/Flann/Linux/include")
        endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    endif()
endmacro()

# Include HDILib includes via target_include_directories to the current project
macro(set_HDILib_project_includes target)
    if(USE_ARTIFACTORY_LIBS)
        MESSAGE( STATUS "Including PREBUILT HDILib ${HDILib_INCLUDE_DIR}")
        target_include_directories("${target}" PRIVATE "${HDILib_INCLUDE_DIR}")
    else()
    	if(UNIX AND NOT APPLE)
            target_include_directories("${target}" PRIVATE "${HDILIB_ROOT}/include")
        else()
            target_include_directories("${target}" PRIVATE "${LIBRARY_INSTALL_DIR}/tSNE/lib/HDI/include")
        endif()
    endif()
endmacro()

# Link the flann libraries via target_link_library to the current project
macro(set_flann_project_link_libraries target)
    if(USE_ARTIFACTORY_LIBS)
        MESSAGE( STATUS "Linking PREBUILT flann libraries...")
        if(MSVC)
            target_link_libraries("${target}" debug "${LIBRARY_INSTALL_DIR}/flann/lib/Debug/flann.lib")
            target_link_libraries("${target}" optimized "${LIBRARY_INSTALL_DIR}/flann/lib/Release/flann.lib")
        endif()
    else()
        if(MSVC)
            MESSAGE( STATUS "Linking Windows flann libraries...")
            target_link_libraries("${target}" debug "${LIBRARY_INSTALL_DIR}/tSNE/lib/Flann/Win/Debug/flann_cpp_s.lib")
            target_link_libraries("${target}" optimized "${LIBRARY_INSTALL_DIR}/tSNE/lib/Flann/Win/Release/flann_cpp_s.lib")
        endif()

        if(APPLE)
            MESSAGE( STATUS "Linking Mac OS X libraries...")
            target_link_libraries("${target}" "${LIBRARY_INSTALL_DIR}/tSNE/lib/Flann/OSX/libflann_s.a")
        endif()
    endif()
endmacro()

# Link the HDILib libraries via target_link_library to the current project
macro(set_HDILib_project_link_libraries target)
    if(USE_ARTIFACTORY_LIBS)
        MESSAGE( STATUS "Linking PREBUILT HDILib libraries...")
        target_link_libraries("${target}" ${HDILib_LINK_LIBS})
    else()
        if(MSVC)
            MESSAGE( STATUS "Linking Windows HDILib libraries...")
            target_link_libraries("${target}" debug "${LIBRARY_INSTALL_DIR}/tSNE/lib/HDI/Win/Debug/hdidimensionalityreduction.lib")
            target_link_libraries("${target}" debug "${LIBRARY_INSTALL_DIR}/tSNE/lib/HDI/Win/Debug/hdidata.lib")
            target_link_libraries("${target}" debug "${LIBRARY_INSTALL_DIR}/tSNE/lib/HDI/Win/Debug/hdiutils.lib")
            target_link_libraries("${target}" optimized "${LIBRARY_INSTALL_DIR}/tSNE/lib/HDI/Win/Release/hdidimensionalityreduction.lib")
            target_link_libraries("${target}" optimized "${LIBRARY_INSTALL_DIR}/tSNE/lib/HDI/Win/Release/hdidata.lib")
            target_link_libraries("${target}" optimized "${LIBRARY_INSTALL_DIR}/tSNE/lib/HDI/Win/Release/hdiutils.lib")
            target_link_libraries("${target}" debug "${LIBRARY_INSTALL_DIR}/tSNE/lib/Flann/Win/Debug/flann_cpp_s.lib")
            target_link_libraries("${target}" optimized "${LIBRARY_INSTALL_DIR}/tSNE/lib/Flann/Win/Release/flann_cpp_s.lib")
        endif()
        if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
            MESSAGE( STATUS "Linking Linux libraries: HDILib at ${HDILIB_ROOT}")
            target_link_libraries("${target}" HDI::hdidimensionalityreduction HDI::hdiutils HDI::hdidata)
        endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        if(APPLE)
            MESSAGE( STATUS "Linking Mac OS X HDILib libraries...")
            target_link_libraries("${target}" debug "${LIBRARY_INSTALL_DIR}/tSNE/lib/AtSNE/OSX/Debug/libbh_t_sne_library.a")
            target_link_libraries("${target}" optimized "${LIBRARY_INSTALL_DIR}/tSNE/lib/AtSNE/OSX/Release/libbh_t_sne_library.a")
        endif()
    endif()
endmacro()

# lz4 currently only with prebuilt libs - is used in flann 1.8.4 and greater.
macro(set_lz4_project_includes target)
    if(USE_ARTIFACTORY_LIBS)
        MESSAGE( STATUS "Including PREBUILT lz4 ${LIBRARY_INSTALL_DIR}/lz4")
        target_include_directories("${target}" PRIVATE "${LIBRARY_INSTALL_DIR}/lz4/Release/include")
    else()
    endif()
endmacro()

macro(set_lz4_project_link_libraries target)
    if(USE_ARTIFACTORY_LIBS)
        MESSAGE( STATUS "Linking PREBUILT lz4 libraries...")
        if(MSVC)
            target_link_libraries("${target}" debug "${LIBRARY_INSTALL_DIR}/lz4/Debug/lib/lz4.lib")
            target_link_libraries("${target}" optimized "${LIBRARY_INSTALL_DIR}/lz4/Release/lib/lz4.lib")
        endif()
        if(APPLE)
            target_link_libraries("${target}" debug "${LIBRARY_INSTALL_DIR}/lz4/Debug/lib/liblz4.a")
            target_link_libraries("${target}" optimized "${LIBRARY_INSTALL_DIR}/lz4/Release/lib/liblz4.a")        
        endif()
    else()
    endif()
endmacro()

macro(set_omp_project_link_libraries target)
    if(USE_ARTIFACTORY_LIBS)

        if(APPLE)
            MESSAGE( STATUS "Linking PREBUILT OpenMP libraries...")
            target_link_libraries("${target}" debug "/usr/local/lib/libomp.dylib")
            target_link_libraries("${target}" optimized "/usr/local/lib/libomp.dylib")        
        endif()
    else()
    endif()
endmacro()
