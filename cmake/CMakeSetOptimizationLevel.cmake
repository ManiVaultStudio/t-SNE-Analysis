# -----------------------------------------------------------------------------
# Sets the optimization level
# -----------------------------------------------------------------------------
macro(set_optimization_level target level)
    message(STATUS "Set optimization level in release for ${target} to ${level}")

    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
        set(OPTIMIZATION_LEVEL_FLAG "-O${level}")
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        if(${level} EQUAL 0)
            set(OPTIMIZATION_LEVEL_FLAG "/Od")
        else()
            set(OPTIMIZATION_LEVEL_FLAG "/O${level}")
        endif()
    endif()

    target_compile_options(${target} PRIVATE "$<$<CONFIG:Release>:${OPTIMIZATION_LEVEL_FLAG}>")

    message( STATUS "Optimization level for ${target} (release) is ${OPTIMIZATION_LEVEL_FLAG}")
endmacro()
