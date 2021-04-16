cmake_minimum_required(VERSION 3.15.0)

set(HDILib_VERSION multi_branch)

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
# Normalize the incoming install path
file(TO_CMAKE_PATH $ENV{HDPS_INSTALL_DIR} INSTALL_DIR)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set(UI_FILES
    ui/DimensionSelectionWidget.ui
)

QT5_WRAP_UI(UI_HEADERS ${UI_FILES})

include(InstallArtifactoryPackage)
install_artifactory_package(HDILib ${HDILib_VERSION} biovault TRUE) #1.2.3
find_package(HDILib COMPONENTS hdidimensionalityreduction hdiutils hdidata PATHS ${CMAKE_MODULE_PATH} REQUIRED)

install_artifactory_package(flann ${flann_VERSION} lkeb FALSE) #1.2.3
find_package(flann PATHS ${CMAKE_MODULE_PATH} REQUIRED)

include_directories("${INSTALL_DIR}/$<CONFIGURATION>/include/")
include_directories (${HDILib_INCLUDE_DIR})

if(MSVC)
    include_directories ("lib/Flann/Win/include")

    MESSAGE( STATUS "Linking Windows libraries...")
    link_libraries(${HDILib_LINK_LIBS})
    link_libraries(debug "${CMAKE_CURRENT_SOURCE_DIR}/tSNE/lib/Flann/Win/Debug/flann_cpp_s.lib")
    link_libraries(optimized "${CMAKE_CURRENT_SOURCE_DIR}/tSNE/lib/Flann/Win/Release/flann_cpp_s.lib")
endif(MSVC)

if(APPLE)
    include_directories ("lib/Flann/OSX/include")
endif(APPLE)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    include_directories ("lib/Flann/Linux/include")
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

include(CMakeCommon)
include(CMakeTsneProject)
include(CMakeHsneProject)
