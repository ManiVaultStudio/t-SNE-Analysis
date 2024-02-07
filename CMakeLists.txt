cmake_minimum_required(VERSION 3.15.0)

# -----------------------------------------------------------------------------
# CMake Options
# -----------------------------------------------------------------------------
set(VERBOSE TRUE)

# Set the directory where CMake include files are to be foundLIBRA
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# -----------------------------------------------------------------------------
# Library versions
# -----------------------------------------------------------------------------
set(HDILib_VERSION 1.2.6)
set(flann_VERSION 1.9.2)
set(lz4_VERSION 1.9.3)

# -----------------------------------------------------------------------------
# Define the SNE Analyses project
# -----------------------------------------------------------------------------
project("SNE-Analyses" C CXX)

if(NOT USE_ARTIFACTORY_LIBS)
    set(USE_ARTIFACTORY_LIBS TRUE CACHE BOOL "Use the prebuilt libraries from artifactory")
endif()

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
# If so, sets the CMake Install Prefix to the HDPS install directory.
# This if-statement must be placed after the project() command to work.
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX ${MV_INSTALL_DIR} CACHE PATH "..." FORCE)
endif()

if(NOT USE_ARTIFACTORY_LIBS)
    message(STATUS "***********NO PBL*********** ${USE_ARTIFACTORY_LIBS}")
endif()

message(STATUS "MV_INSTALL_DIR set to ${MV_INSTALL_DIR}")

# -----------------------------------------------------------------------------
# Windows compiler and linker flags
# -----------------------------------------------------------------------------
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DWIN32 /EHsc /MP  /permissive- /Zc:__cplusplus")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
endif(MSVC)

# -----------------------------------------------------------------------------
# Qt library and flags
# -----------------------------------------------------------------------------
# Find Qt library
find_package(Qt6 6.3.1 COMPONENTS Widgets WebEngineWidgets REQUIRED)

# Enables Qt auto-moc to run for all target projects
set(CMAKE_AUTOMOC ON)

# -----------------------------------------------------------------------------
# Install libraries
# -----------------------------------------------------------------------------
set(LIBRARY_INSTALL_DIR ${PROJECT_BINARY_DIR})


include(InstallArtifactoryPackage)
if (USE_ARTIFACTORY_LIBS AND NOT ARTIFACTORY_LIBS_INSTALLED) 
    message(STATUS "Installing artifactory packages to: ${LIBRARY_INSTALL_DIR}")
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

if(USE_ARTIFACTORY_LIBS AND ARTIFACTORY_LIBS_INSTALLED)
    # If the HDILib_VERSION is not a x.y.z version number 
    # omit the version argument from find package
    find_package(HDILib COMPONENTS hdidimensionalityreduction hdiutils hdidata PATHS "${LIBRARY_INSTALL_DIR}/HDILib" CONFIG REQUIRED)
else()
    if(NOT HDILIB_ROOT)
        message(FATAL_ERROR "Define HDILIB_ROOT")
    endif()
    find_package(HDILib COMPONENTS hdidimensionalityreduction hdiutils hdidata PATHS "${HDILIB_ROOT}" CONFIG REQUIRED)
    
    if(HDILib_FOUND)
        message(STATUS "HDILib found at ${HDILIB_ROOT}") 
    endif()
    
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

# Automatically set the debug environment (command + working directory) for MSVC
if(MSVC)
    set_property(TARGET ${PROJECT} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY $<IF:$<CONFIG:DEBUG>,${MV_INSTALL_DIR}/debug,${MV_INSTALL_DIR}/release>)
    set_property(TARGET ${PROJECT} PROPERTY VS_DEBUGGER_COMMAND $<IF:$<CONFIG:DEBUG>,"${MV_INSTALL_DIR}/debug/ManiVault Studio.exe","${MV_INSTALL_DIR}/release/ManiVault Studio.exe">)
endif()