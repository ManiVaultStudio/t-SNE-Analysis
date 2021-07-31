cmake_minimum_required(VERSION 3.15.0)

# -----------------------------------------------------------------------------
# CMake Options
# -----------------------------------------------------------------------------
set(VERBOSE TRUE)

# Set the directory where CMake include files are to be found
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# -----------------------------------------------------------------------------
# Library versions
# -----------------------------------------------------------------------------
set(HDILib_VERSION latest)
set(flann_VERSION 1.8.5)
if(APPLE)
    set(flann_VERSION 1.9.1)
endif()

# -----------------------------------------------------------------------------
# Define the SNE Analyses project
# -----------------------------------------------------------------------------
project("SNE-Analyses" C CXX)

# -----------------------------------------------------------------------------
# Set install directory
# -----------------------------------------------------------------------------
# Checks whether this is the first CMake run for this project.
# If so, sets the CMake Install Prefix to the HDPS install directory.
# This if-statement must be placed after the project() command to work.
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    if (NOT DEFINED ENV{HDPS_INSTALL_DIR})
        message(WARNING "Environment variable HDPS_INSTALL_DIR not set. \
                        Plugins will have to be manually copied or \
                        CMAKE_INSTALL_PREFIX should be set manually.")
    else()
        # Load HDPS_INSTALL_DIR in CMake format and store in INSTALL_DIR
        file(TO_CMAKE_PATH $ENV{HDPS_INSTALL_DIR} INSTALL_DIR)
        # Set the default install prefix to HDPS_INSTALL_DIR
        set(CMAKE_INSTALL_PREFIX ${INSTALL_DIR} CACHE PATH "..." FORCE)
    endif()
endif()
set(INSTALL_DIR ${CMAKE_INSTALL_PREFIX})

# -----------------------------------------------------------------------------
# Windows compiler and linker flags
# -----------------------------------------------------------------------------
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DWIN32 /EHsc /MP")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
endif(MSVC)

# -----------------------------------------------------------------------------
# Qt library and flags
# -----------------------------------------------------------------------------
# Find Qt library
find_package(Qt5 COMPONENTS Widgets WebEngineWidgets REQUIRED)

# Enables Qt auto-moc to run for all target projects
set(CMAKE_AUTOMOC ON)

# -----------------------------------------------------------------------------
# Install libraries
# -----------------------------------------------------------------------------
set(LIBRARY_INSTALL_DIR ${PROJECT_BINARY_DIR})

if(NOT USE_PREBUILT_LIBS)
    set(USE_PREBUILT_LIBS TRUE CACHE BOOL "Use the prebuilt libraries from artifactory")
endif()

include(InstallArtifactoryPackage)
if (USE_PREBUILT_LIBS AND NOT PREBUILT_LIBS_INSTALLED)
    # Both HDILib and flann are available prebuilt in the lkeb-artifactory.
    # The packages can be downloaded containing both Debug & Release binaries
    install_artifactory_package(HDILib ${HDILib_VERSION} biovault TRUE) #1.2.3
    install_artifactory_package(flann ${flann_VERSION} lkeb TRUE) #1.2.3

    message(STATUS "module path ${CMAKE_MODULE_PATH}")
    message(STATUS "Call find_package HDILib search ${CMAKE_SOURCE_DIR}/HDILib/lib/cmake/HDILib")
    message(STATUS "Run find_package HDILib")

    set(PREBUILT_LIBS_INSTALLED TRUE CACHE BOOL "Use the prebuilt libraries from artifactory" FORCE)
endif()

if(USE_PREBUILT_LIBS AND PREBUILT_LIBS_INSTALLED)
    # If the HDILib_VERSION is not a x.y.z version number 
    # omit the version argument from find package
    find_package(HDILib COMPONENTS hdidimensionalityreduction hdiutils hdidata PATHS "${LIBRARY_INSTALL_DIR}/HDILib" CONFIG REQUIRED)
endif()

find_package(OpenGL REQUIRED)

# -----------------------------------------------------------------------------
# Target definitions
# -----------------------------------------------------------------------------
if (APPLE)
    # This silences OpenGL deprecation warnings on MacOS
    add_definitions(-DGL_SILENCE_DEPRECATION)
endif()

# -----------------------------------------------------------------------------
# Project targets
# -----------------------------------------------------------------------------
include(CMakeCommon)
include(CMakeTsneProject)
include(CMakeHsneProject)
