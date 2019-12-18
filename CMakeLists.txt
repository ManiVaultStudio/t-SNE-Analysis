cmake_minimum_required(VERSION 3.1.0)

set(PROJECT "TsneAnalysisPlugin")

PROJECT(${PROJECT})

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DWIN32 /EHsc /MP")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:LIBCMT")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
endif(MSVC)

if (APPLE)
    add_definitions(-DGL_SILENCE_DEPRECATION)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.12" CACHE STRING "Minimum OS X deployment version")
    
    find_program(BREW NAMES brew)
    if("${BREW}" STRGREATER "")
        message(STATUS "Running on macOS Darwin using ${BREW}")
        execute_process(COMMAND ${BREW} --prefix llvm OUTPUT_VARIABLE BREW_LLVM_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)
        execute_process(COMMAND ${BREW} --prefix flann OUTPUT_VARIABLE BREW_FLANN_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)
    endif("${BREW}" STRGREATER "")
    
    find_package(LLVM REQUIRED CONFIG HINTS "${BREW_LLVM_PREFIX}/lib/cmake/llvm")
    if(EXISTS ${LLVM_INSTALL_PREFIX})
        message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION} in ${LLVM_INSTALL_PREFIX}")

        find_package(Clang REQUIRED CONFIG HINTS "${BREW_LLVM_PREFIX}/lib/cmake/clang")
        set(CMAKE_C_COMPILER "${LLVM_INSTALL_PREFIX}/bin/clang")
        set(CMAKE_CXX_COMPILER "${LLVM_INSTALL_PREFIX}/bin/clang++")
        set(CMAKE_XCODE_ATTRIBUTE_CC "${LLVM_INSTALL_PREFIX}/bin/clang")
        set(CMAKE_XCODE_ATTRIBUTE_CXX "${LLVM_INSTALL_PREFIX}/bin/clang++")
        set(CMAKE_XCODE_ATTRIBUTE_COMPILER_INDEX_STORE_ENABLE "NO")
    
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
        include_directories(${LLVM_INSTALL_PREFIX}/include/clang/Tooling)
    else()
        message(WARNING "Could not find custom llvm using base install.")
    endif(EXISTS ${LLVM_INSTALL_PREFIX})
endif(APPLE)

find_package(Qt5 COMPONENTS Widgets WebEngineWidgets REQUIRED)

set(SOURCES
    src/ModelResetter.h
    src/ModelResetter.cpp
    src/DimensionSelectionHolder.h
    src/DimensionSelectionHolder.cpp
    src/DimensionSelectionItemModel.h
    src/DimensionSelectionItemModel.cpp
    src/DimensionSelectionProxyModel.h
    src/DimensionSelectionProxyModel.cpp
    src/DimensionSelectionWidget.h
    src/DimensionSelectionWidget.cpp
    src/TsneAnalysisPlugin.h
    src/TsneAnalysisPlugin.cpp
    src/TsneAnalysisPlugin.json
    src/TsneAnalysis.h
    src/TsneAnalysis.cpp
    src/TsneSettingsWidget.h
    src/TsneSettingsWidget.cpp
    src/TsneData.h
)

source_group( Plugin FILES ${SOURCES})

set(UI_FILES
    ui/DimensionSelectionWidget.ui
)

QT5_WRAP_UI(UI_HEADERS ${UI_FILES})

include_directories("$ENV{HDPS_INSTALL_DIR}/$<CONFIGURATION>/include/")

if(MSVC)
    include_directories ("lib/HDI/include")
    include_directories ("lib/Flann/Win/include")
endif(MSVC)

if(APPLE)
    include_directories ("${CMAKE_CURRENT_SOURCE_DIR}/lib/HDI/macOS/include")
    if(EXISTS ${BREW_FLANN_PREFIX})
        message(STATUS "Flann found from brew at ${BREW_FLANN_PREFIX}.")
        include_directories("${BREW_FLANN_PREFIX}/include")
        set(FLANN_PREFIX ${BREW_FLANN_PREFIX})
    else()
        # there should probably be an alternative in here for setting the flann path
        message(WARNING "Flann not found.")
        set(FLANN_PREFIX)
    endif(EXISTS ${BREW_FLANN_PREFIX})
endif(APPLE)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    include_directories ("lib/Flann/Linux/include")
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

add_library(${PROJECT} SHARED ${SOURCES} ${UI_FILES})

target_compile_definitions(${PROJECT} PRIVATE QT_MESSAGELOGCONTEXT)

# Request C++17, in order to use std::for_each_n with std::execution::par_unseq.
set_property(TARGET ${PROJECT} PROPERTY CXX_STANDARD 17)

target_link_libraries(${PROJECT} Qt5::Widgets)
target_link_libraries(${PROJECT} Qt5::WebEngineWidgets)
if(APPLE)
    target_link_libraries(${PROJECT} "$ENV{HDPS_INSTALL_DIR}/$<CONFIGURATION>/HDPS.app/Contents/Frameworks/${CMAKE_SHARED_LIBRARY_PREFIX}HDPS_Public${CMAKE_SHARED_LIBRARY_SUFFIX}")
    target_link_libraries(${PROJECT} "$ENV{HDPS_INSTALL_DIR}/$<CONFIGURATION>/Plugins/${CMAKE_SHARED_LIBRARY_PREFIX}PointData${CMAKE_SHARED_LIBRARY_SUFFIX}")
else()
    target_link_libraries(${PROJECT} "$ENV{HDPS_INSTALL_DIR}/$<CONFIGURATION>/lib/HDPS_Public.lib")
    target_link_libraries(${PROJECT} "$ENV{HDPS_INSTALL_DIR}/$<CONFIGURATION>/lib/PointData.lib")
endif(APPLE)

if(MSVC)
    MESSAGE( STATUS "Linking Windows libraries...")
    target_link_libraries(${PROJECT} debug "${CMAKE_CURRENT_SOURCE_DIR}/lib/HDI/Win/Debug/hdidimensionalityreduction.lib")
    target_link_libraries(${PROJECT} debug "${CMAKE_CURRENT_SOURCE_DIR}/lib/HDI/Win/Debug/hdidata.lib")
    target_link_libraries(${PROJECT} debug "${CMAKE_CURRENT_SOURCE_DIR}/lib/HDI/Win/Debug/hdiutils.lib")
    target_link_libraries(${PROJECT} optimized "${CMAKE_CURRENT_SOURCE_DIR}/lib/HDI/Win/Release/hdidimensionalityreduction.lib")
    target_link_libraries(${PROJECT} optimized "${CMAKE_CURRENT_SOURCE_DIR}/lib/HDI/Win/Release/hdidata.lib")
    target_link_libraries(${PROJECT} optimized "${CMAKE_CURRENT_SOURCE_DIR}/lib/HDI/Win/Release/hdiutils.lib")

    target_link_libraries(${PROJECT} debug "${CMAKE_CURRENT_SOURCE_DIR}/lib/Flann/Win/Debug/flann_cpp_s.lib")
    target_link_libraries(${PROJECT} optimized "${CMAKE_CURRENT_SOURCE_DIR}/lib/Flann/Win/Release/flann_cpp_s.lib")
endif(MSVC)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    MESSAGE( STATUS "Linking Linux libraries...")
    # TODO
    #target_link_libraries(${PROJECT} "${CMAKE_CURRENT_SOURCE_DIR}/lib/AtSNE/Linux/libbh_t_sne_library.a")
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

if(APPLE)
    MESSAGE( STATUS "Linking Mac OS X libraries...")

    FIND_LIBRARY(OpenGL_LIBRARY OpenGL )
    target_link_libraries(${PROJECT} ${OpenGL_LIBRARY})
    
    execute_process(COMMAND ${BREW} --prefix libomp OUTPUT_VARIABLE BREW_LIBOMP_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(EXISTS ${BREW_LIBOMP_PREFIX})
        target_link_libraries(${PROJECT} "${BREW_LIBOMP_PREFIX}/lib/libomp.dylib")
    else()
        message(WARNING "Brew found, but OpenMP support could not be detected, using Grand Central Dispatch instead.")
        add_definitions( -D__USE_GCD__)
    endif(EXISTS ${BREW_LIBOMP_PREFIX})

	target_link_libraries(${PROJECT} debug "${CMAKE_CURRENT_SOURCE_DIR}/lib/HDI/macOS/Debug/libhdidata.a")
	target_link_libraries(${PROJECT} optimized "${CMAKE_CURRENT_SOURCE_DIR}/lib/HDI/macOS/Release/libhdidata.a")
	target_link_libraries(${PROJECT} debug "${CMAKE_CURRENT_SOURCE_DIR}/lib/HDI/macOS/Debug/libhdidimensionalityreduction.a")
	target_link_libraries(${PROJECT} optimized "${CMAKE_CURRENT_SOURCE_DIR}/lib/HDI/macOS/Release/libhdidimensionalityreduction.a")
	target_link_libraries(${PROJECT} debug "${CMAKE_CURRENT_SOURCE_DIR}/lib/HDI/macOS/Debug/libhdiutils.a")
	target_link_libraries(${PROJECT} optimized "${CMAKE_CURRENT_SOURCE_DIR}/lib/HDI/macOS/Release/libhdiutils.a")

    if(EXISTS ${FLANN_PREFIX})
        target_link_libraries(${PROJECT} "${FLANN_PREFIX}/lib/libflann_s.a")
    endif(EXISTS ${FLANN_PREFIX})
endif(APPLE)

add_custom_command(TARGET ${PROJECT} POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "$<TARGET_FILE:${PROJECT}>"
    "$ENV{HDPS_INSTALL_DIR}/$<CONFIGURATION>/Plugins/$<TARGET_FILE_NAME:${PROJECT}>"
)