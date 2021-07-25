cmake_minimum_required(VERSION 3.15.0)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX $ENV{HDPS_INSTALL_DIR} CACHE PATH "..." FORCE)
endif()

set(HDILib_VERSION latest)
set(VERBOSE TRUE)

if(NOT USE_PREBUILT_LIBS)
    set(USE_PREBUILT_LIBS TRUE CACHE BOOL "Use the prebuilt libraries from artifactory")
endif()

if(WIN32)
    set(flann_VERSION 1.8.5)
elseif(APPLE)
    set(flann_VERSION 1.9.1)
endif()

project("SNE-Analyses")
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DWIN32 /EHsc /MP")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:LIBCMT")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
endif(MSVC)

find_package(Qt5 COMPONENTS Widgets WebEngineWidgets REQUIRED)

set(INSTALL_DIR ${CMAKE_SOURCE_DIR} CACHE PATH "Canonical form of HDPS_INSTALL_DIR")

# Normalize the incoming install path
file(TO_CMAKE_PATH $ENV{HDPS_INSTALL_DIR} INSTALL_DIR_TEMP)

set(INSTALL_DIR ${INSTALL_DIR_TEMP} CACHE PATH "" FORCE)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set(UI_FILES
    ui/DimensionSelectionWidget.ui
)

QT5_WRAP_UI(UI_HEADERS ${UI_FILES})

include(InstallArtifactoryPackage)
if (USE_PREBUILT_LIBS AND NOT PREBUILT_LIBS_INSTALLED)
    # Both HDILib and flann are available prebuilt in the lkeb-artifactory.
    # The packages can be downloaded containing both Debug & Release binaries
    install_artifactory_package(HDILib ${HDILib_VERSION} biovault TRUE) #1.2.3
    install_artifactory_package(flann ${flann_VERSION} lkeb TRUE) #1.2.3

    message("module path ${CMAKE_MODULE_PATH}")
    message("Call find_package HDILib search ${CMAKE_SOURCE_DIR}/HDILib/lib/cmake/HDILib")
    message("Run find_package HDILib")

    set(PREBUILT_LIBS_INSTALLED TRUE CACHE BOOL "Use the prebuilt libraries from artifactory" FORCE)
endif()

if(USE_PREBUILT_LIBS AND PREBUILT_LIBS_INSTALLED)
    # If the HDILib_VERSION is not a x.y.z version number 
    # omit the version argument from find package
    find_package(HDILib COMPONENTS hdidimensionalityreduction hdiutils hdidata PATHS "${INSTALL_DIR}/HDILib" CONFIG REQUIRED)
endif()

if (APPLE)
    # This silences OpenGL deprecation warnings on MacOS
    add_definitions(-DGL_SILENCE_DEPRECATION)
endif()

include(CMakeCommon)
include(CMakeTsneProject)
include(CMakeHsneProject)

add_custom_command(TARGET TsneAnalysisPlugin POST_BUILD
	COMMAND "${CMAKE_COMMAND}"
	--install ${CMAKE_BINARY_DIR}
	--config $<CONFIGURATION>
	--prefix ${INSTALL_DIR}/$<CONFIGURATION>
)