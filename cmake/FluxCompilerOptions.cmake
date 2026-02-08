# ============================================================================
# FluxCompilerOptions.cmake
# Common compiler options for all Flux compiler targets
# ============================================================================

# Macro to apply Flux compiler options to a target
macro(flux_set_target_options target_name)
    if(MSVC)
        target_compile_options(${target_name} PRIVATE
            /W4             # High warning level
            /WX-            # Don't treat warnings as errors (for now)
            /permissive-    # Standards conformance mode
            /Zc:__cplusplus # Report correct __cplusplus value
            /Zc:preprocessor # Use standards-conforming preprocessor
            /utf-8          # Source and execution charset UTF-8
            /EHsc           # Standard C++ exceptions
        )

        # LLVM is built without RTTI by default on Windows
        # We match that for our LLVM-interfacing code
        target_compile_options(${target_name} PRIVATE /GR-)

        # Debug-specific flags
        target_compile_options(${target_name} PRIVATE
            $<$<CONFIG:Debug>:/Od /Zi /RTC1>
            $<$<CONFIG:Release>:/O2 /DNDEBUG>
            $<$<CONFIG:RelWithDebInfo>:/O2 /Zi /DNDEBUG>
        )

        # Platform definitions
        target_compile_definitions(${target_name} PRIVATE
            _CRT_SECURE_NO_WARNINGS
            _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
            NOMINMAX    # Prevent Windows.h min/max macros
            WIN32_LEAN_AND_MEAN
        )
    else()
        target_compile_options(${target_name} PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Wno-unused-parameter
            -fno-rtti      # Match LLVM's no-RTTI default
        )

        target_compile_options(${target_name} PRIVATE
            $<$<CONFIG:Debug>:-O0 -g>
            $<$<CONFIG:Release>:-O2 -DNDEBUG>
            $<$<CONFIG:RelWithDebInfo>:-O2 -g -DNDEBUG>
        )

        # AddressSanitizer
        if(FLUX_ENABLE_ASAN AND CMAKE_BUILD_TYPE STREQUAL "Debug")
            target_compile_options(${target_name} PRIVATE -fsanitize=address)
            target_link_options(${target_name} PRIVATE -fsanitize=address)
        endif()
    endif()
endmacro()
