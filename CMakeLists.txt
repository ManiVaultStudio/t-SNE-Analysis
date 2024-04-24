cmake_minimum_required(VERSION 3.15.0)

project("SNE-Analyses" C CXX)

# -----------------------------------------------------------------------------
# User options
# -----------------------------------------------------------------------------
option(USE_ARTIFACTORY_LIBS "Use the prebuilt libraries from artifactory" ON)
option(ENABLE_AVX "Enable AVX support" OFF)
option(MV_UNITY_BUILD "Combine target source files into batches for faster compilation" OFF)
set(OPTIMIZATION_LEVEL "2" CACHE STRING "Optimization level for all targets in release builds, e.g. 0, 1, 2")

# -----------------------------------------------------------------------------
# CMake options
# -----------------------------------------------------------------------------
set(VERBOSE TRUE)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# macros for linking both prebuilt artifactory libraries and locally built ones
include(CMakeLinkUtilities)

# Test hardware avx capabilities
include(CMakeSetOptimizationAndAVX)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DWIN32 /EHsc /MP  /permissive- /Zc:__cplusplus")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
endif(MSVC)

# -----------------------------------------------------------------------------
# Set install directory
# -----------------------------------------------------------------------------
# Check if the directory to the ManiVault installation has been provided
if(NOT DEFINED MV_INSTALL_DIR)
    set(MV_INSTALL_DIR "" CACHE PATH "Directory where ManiVault is installed")
    message(FATAL_ERROR "Please set MV_INSTALL_DIR to the directory where ManiVault is installed")
endif()
file(TO_CMAKE_PATH ${MV_INSTALL_DIR} MV_INSTALL_DIR)

# Checks whether this is the first CMake run for this project.
# If so, sets the CMake Install Prefix to the ManiVault install directory.
# This if-statement must be placed after the project() command to work.
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX ${MV_INSTALL_DIR} CACHE PATH "..." FORCE)
endif()

message(STATUS "MV_INSTALL_DIR set to ${MV_INSTALL_DIR}")

# -----------------------------------------------------------------------------
# Install dependency libraries
# -----------------------------------------------------------------------------

set(LIBRARY_INSTALL_DIR ${PROJECT_BINARY_DIR})

include(InstallArtifactoryPackage)
if (USE_ARTIFACTORY_LIBS AND NOT ARTIFACTORY_LIBS_INSTALLED) 
    message(STATUS "Installing artifactory packages to: ${LIBRARY_INSTALL_DIR}")

    set(HDILib_VERSION 1.2.6)
    set(flann_VERSION 1.9.2)
    set(lz4_VERSION 1.9.3)

    # Both HDILib and flann are available prebuilt in the lkeb-artifactory as combined Debug/Release packages
    # lz4 is also available in the lkb-artifactory in separate Debug and |Release packages
    install_artifactory_package(HDILib ${HDILib_VERSION} biovault TRUE) 
    install_artifactory_package(flann ${flann_VERSION} lkeb TRUE)
    install_artifactory_package(lz4 ${lz4_VERSION} _ FALSE)

    message(STATUS "module path ${CMAKE_MODULE_PATH}")
    message(STATUS "Call find_package HDILib search ${CMAKE_SOURCE_DIR}/HDILib/lib/cmake/HDILib")
    message(STATUS "Run find_package HDILib")

    set(ARTIFACTORY_LIBS_INSTALLED TRUE CACHE BOOL "Use the prebuilt libraries from artifactory" FORCE)
endif()

# -----------------------------------------------------------------------------
# Dependencies
# -----------------------------------------------------------------------------

if(USE_ARTIFACTORY_LIBS AND ARTIFACTORY_LIBS_INSTALLED)
    # If the HDILib_VERSION is not a x.y.z version number 
    # omit the version argument from find package
    find_package(HDILib COMPONENTS hdidimensionalityreduction hdiutils hdidata PATHS "${LIBRARY_INSTALL_DIR}/HDILib" CONFIG REQUIRED)
else()
    if(NOT HDILIB_ROOT)
        message(FATAL_ERROR "Define HDILIB_ROOT")
    endif()

    find_package(flann CONFIG REQUIRED)
    find_package(HDILib COMPONENTS hdidimensionalityreduction hdiutils hdidata PATHS "${HDILIB_ROOT}" CONFIG REQUIRED)

    message(STATUS "HDILib found at ${HDILIB_ROOT} with ${HDILib_LINK_LIBS}")    
endif()

find_package(Qt6 6.3.1 COMPONENTS Widgets WebEngineWidgets REQUIRED)

find_package(OpenGL REQUIRED)
find_package(OpenMP)

if(OpenMP_CXX_FOUND)
    message(STATUS "Found OpenMP")
endif()

if(UNIX)
    message(STATUS "pThreads for Linux")
    find_package(Threads REQUIRED)
endif(UNIX)

set(THIRD_PARTY_JSON
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/json/nlohmann/json.hpp
)

# -----------------------------------------------------------------------------
# Project targets
# -----------------------------------------------------------------------------

# Source files shared by both Hsne and Tsne plugins
add_subdirectory(src/Common)

include(CMakeTsneProject)
include(CMakeHsneProject)
