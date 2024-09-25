# The following 4 macros are designed to support the use of user supplied
# libraries or the PREBUILT libraries for flann and HDILib from the LKEB
# artifactory.

# Link the flann libraries via target_link_library to the current project
macro(set_flann_project_link_libraries target)
    if(TARGET flann::flann_cpp_s)
        set(FLANN_TARGET flann::flann_cpp_s)
    elseif(TARGET flann::flann_cpp)
        set(FLANN_TARGET flann::flann_cpp)
    elseif(TARGET flann::flann_s)
        set(FLANN_TARGET flann::flann_s)
    elseif(TARGET flann::flann)
        set(FLANN_TARGET flann::flann)
    else()
        message(FATAL_ERROR "No Flann target found.")
    endif()

    message (STATUS "Linking Flann library " ${FLANN_TARGET})
    target_link_libraries(${target} PRIVATE ${FLANN_TARGET})
endmacro()

# Include HDILib includes via target_include_directories to the current project
macro(set_HDILib_project_includes target)
    MESSAGE( STATUS "Including HDILib ${HDILib_INCLUDE_DIR}")
    target_include_directories("${target}" PRIVATE "${HDILib_INCLUDE_DIR}")
endmacro()

# Link the HDILib libraries via target_link_library to the current project
macro(set_HDILib_project_link_libraries target)
    MESSAGE( STATUS "Linking HDILib libraries ${HDILib_LINK_LIBS}")
    target_link_libraries("${target}" PRIVATE ${HDILib_LINK_LIBS})
endmacro()

macro(set_lz4_project_link_libraries target)
    MESSAGE( STATUS "Linking lz4")
    target_link_libraries("${target}" PRIVATE lz4::lz4)
endmacro()

# This silences OpenGL deprecation warnings on MacOS
macro(silence_opengl_deprecation target)
    if (APPLE)
        target_compile_definitions("${target}" PRIVATE GL_SILENCE_DEPRECATION)
    endif()
endmacro()
