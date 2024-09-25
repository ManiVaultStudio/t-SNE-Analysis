# The following 4 macros are designed to support the use of user supplied
# libraries or the PREBUILT libraries for flann and HDILib from the LKEB
# artifactory.

# Link the flann libraries via target_link_library to the current project
macro(set_flann_project_link_libraries target)

    # prefer static linking
    if(NOT FLANN_TARGET)
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

    # prefer static linking
    if(NOT LZ4_TARGET)
        if(TARGET LZ4::lz4_static)
            set(LZ4_TARGET LZ4::lz4_static)
        elseif(TARGET LZ4::lz4_shared)
            set(LZ4_TARGET LZ4::lz4_shared)
        elseif(TARGET lz4::lz4)
            set(LZ4_TARGET lz4::lz4)
        elseif(TARGET LZ4::lz4)    # intentionally UPPERCASE::LOWERCASE
            set(LZ4_TARGET LZ4::lz4)
        else()
            message(FATAL_ERROR "No LZ4 target found.")
        endif()
    endif()

MESSAGE( STATUS "Linking lz4 library " ${LZ4_TARGET})
    target_link_libraries("${target}" PRIVATE ${LZ4_TARGET})
endmacro()

# This silences OpenGL deprecation warnings on MacOS
macro(silence_opengl_deprecation target)
    if (APPLE)
        target_compile_definitions("${target}" PRIVATE GL_SILENCE_DEPRECATION)
    endif()
endmacro()
