cmake_minimum_required(VERSION 3.20)
project(FluxCompiler VERSION 0.1.0 LANGUAGES CXX)

# ============================================================================
# C++20 Configuration
# ============================================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ============================================================================
# Build Options
# ============================================================================
option(FLUX_ENABLE_TESTS "Enable test builds" ON)
option(FLUX_ENABLE_LIT "Enable LLVM lit tests" ON)
option(FLUX_ENABLE_ASAN "Enable AddressSanitizer (Debug)" OFF)

# ============================================================================
# CMake Module Path
# ============================================================================
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# ============================================================================
# Compiler Flags
# ============================================================================
include(FluxCompilerOptions)

# ============================================================================
# Find LLVM
# ============================================================================
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "LLVM include dirs: ${LLVM_INCLUDE_DIRS}")
message(STATUS "LLVM libraries dir: ${LLVM_LIBRARY_DIRS}")
message(STATUS "LLVM definitions: ${LLVM_DEFINITIONS}")

include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})

# Auto-detect DIA SDK for MSVC (needed by LLVM prebuilts)
if(MSVC)
    set(_DIA_SDK_FOUND FALSE)

    # Try VSINSTALLDIR environment variable first
    if(DEFINED ENV{VSINSTALLDIR})
        set(_DIA_CANDIDATE "$ENV{VSINSTALLDIR}/DIA SDK/lib/amd64")
        cmake_path(NORMAL_PATH _DIA_CANDIDATE)
        if(EXISTS "${_DIA_CANDIDATE}/diaguids.lib")
            link_directories("${_DIA_CANDIDATE}")
            message(STATUS "DIA SDK (from VSINSTALLDIR): ${_DIA_CANDIDATE}")
            set(_DIA_SDK_FOUND TRUE)
        endif()
    endif()

    # Fallback: scan standard VS installation paths
    if(NOT _DIA_SDK_FOUND)
        set(_VS_EDITIONS Community Professional Enterprise BuildTools)
        set(_VS_YEARS 2022 2019)
        set(_VS_ROOTS
            "C:/Program Files/Microsoft Visual Studio"
            "C:/Program Files (x86)/Microsoft Visual Studio"
        )

        foreach(_ROOT IN LISTS _VS_ROOTS)
            foreach(_YEAR IN LISTS _VS_YEARS)
                foreach(_EDITION IN LISTS _VS_EDITIONS)
                    set(_DIA_CANDIDATE "${_ROOT}/${_YEAR}/${_EDITION}/DIA SDK/lib/amd64")
                    if(EXISTS "${_DIA_CANDIDATE}/diaguids.lib")
                        link_directories("${_DIA_CANDIDATE}")
                        message(STATUS "DIA SDK (auto-detected): ${_DIA_CANDIDATE}")
                        set(_DIA_SDK_FOUND TRUE)
                        break()
                    endif()
                endforeach()
                if(_DIA_SDK_FOUND)
                    break()
                endif()
            endforeach()
            if(_DIA_SDK_FOUND)
                break()
            endif()
        endforeach()
    endif()

    if(NOT _DIA_SDK_FOUND)
        message(WARNING "DIA SDK not found — LLVM debug info features may fail to link")
    endif()
endif()
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

# Map LLVM components to library names
llvm_map_components_to_libnames(LLVM_LIBS
    Core
    Support
    IRReader
    BitWriter
    BitReader
    Passes
    ScalarOpts
    InstCombine
    IPO
    Analysis
    TransformUtils
    CodeGen
    Target
    X86CodeGen X86AsmParser X86Desc X86Info
    AArch64CodeGen AArch64AsmParser AArch64Desc AArch64Info
    WebAssemblyCodeGen WebAssemblyAsmParser WebAssemblyDesc WebAssemblyInfo
    MC MCParser
    Object
    DebugInfoDWARF DebugInfoCodeView
)

# Export LLVM libs for subdirectories
set(FLUX_LLVM_LIBS ${LLVM_LIBS} CACHE INTERNAL "LLVM libraries for Flux")
message(STATUS "LLVM libraries: ${FLUX_LLVM_LIBS}")

# ============================================================================
# Project Include Directories
# ============================================================================
include_directories(${CMAKE_SOURCE_DIR}/include)

# ============================================================================
# Compiler Libraries (each is a static library)
# ============================================================================
add_subdirectory(lib/Common)
add_subdirectory(lib/Lexer)
add_subdirectory(lib/AST)
add_subdirectory(lib/Parser)
add_subdirectory(lib/Sema)
add_subdirectory(lib/CodeGen)

# ============================================================================
# Compiler Driver Executable
# ============================================================================
add_subdirectory(tools/flux)

# ============================================================================
# Runtime Library
# ============================================================================
add_subdirectory(runtime)

# ============================================================================
# Tests
# ============================================================================
if(FLUX_ENABLE_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
