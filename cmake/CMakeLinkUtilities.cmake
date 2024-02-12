# The following 4 macros are designed to support the use of user supplied
# libraries or the PREBUILT libraries for flann and HDILib from the LKEB
# artifactory.

# TODO complete for MacOS and Linux

# Include flann includes via target_include_directories to the current project
macro(set_flann_project_includes target)
    if(USE_ARTIFACTORY_LIBS)
        MESSAGE( STATUS "Including PREBUILT ARTIFACTORY flann ${LIBRARY_INSTALL_DIR}/flann")
        target_include_directories("${target}" PRIVATE "${LIBRARY_INSTALL_DIR}/flann/include")
    else()

    endif()
endmacro()

# Link the flann libraries via target_link_library to the current project
macro(set_flann_project_link_libraries target)
    if(USE_ARTIFACTORY_LIBS)
        MESSAGE( STATUS "Linking PREBUILT flann libraries...")
        if(MSVC)
            target_link_libraries("${target}" PRIVATE debug "${LIBRARY_INSTALL_DIR}/flann/lib/Debug/flann.lib")
            target_link_libraries("${target}" PRIVATE optimized "${LIBRARY_INSTALL_DIR}/flann/lib/Release/flann.lib")
        endif()
    else()
        message (STATUS "Linking flann: " ${flann_DIR})
        if(WIN32)
            target_link_libraries("${target}" PRIVATE flann::flann_cpp_s)
        else()
            target_link_libraries("${target}" PRIVATE flann::flann)
        endif()    
    endif()
endmacro()

# Include HDILib includes via target_include_directories to the current project
macro(set_HDILib_project_includes target)
    MESSAGE( STATUS "Including HDILib ${HDILib_INCLUDE_DIR}")
    target_include_directories("${target}" PRIVATE "${HDILib_INCLUDE_DIR}")
endmacro()

# Link the HDILib libraries via target_link_library to the current project
macro(set_HDILib_project_link_libraries target)
    MESSAGE( STATUS "Linking HDILib libraries...")
    target_link_libraries("${target}" PRIVATE ${HDILib_LINK_LIBS})
endmacro()

# lz4 currently only with prebuilt libs - is used in flann 1.8.4 and greater.
macro(set_lz4_project_includes target)
    if(USE_ARTIFACTORY_LIBS)
        if(MSVC)
            MESSAGE( STATUS "Including PREBUILT ARTIFACTORY lz4 ${LIBRARY_INSTALL_DIR}/lz4")
            target_include_directories("${target}" PRIVATE "${LIBRARY_INSTALL_DIR}/lz4/Release/include")
        endif()
    endif()
endmacro()

macro(set_lz4_project_link_libraries target)
    if(USE_ARTIFACTORY_LIBS)
        MESSAGE( STATUS "Linking PREBUILT lz4 libraries...")
        if(MSVC)
            target_link_libraries("${target}" PRIVATE debug "${LIBRARY_INSTALL_DIR}/lz4/Debug/lib/lz4.lib")
            target_link_libraries("${target}" PRIVATE optimized "${LIBRARY_INSTALL_DIR}/lz4/Release/lib/lz4.lib")
        endif()
        if(APPLE)
            target_link_libraries("${target}" debug "${LIBRARY_INSTALL_DIR}/lz4/Debug/lib/liblz4.a")
            target_link_libraries("${target}" optimized "${LIBRARY_INSTALL_DIR}/lz4/Release/lib/liblz4.a")        
        endif()
    endif()
endmacro()

# This silences OpenGL deprecation warnings on MacOS
macro(silence_opengl_deprecation target)
    if (APPLE)
        target_compile_definitions("${target}" PRIVATE GL_SILENCE_DEPRECATION)
    endif()
endmacro()
