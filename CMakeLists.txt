cmake_minimum_required(VERSION 3.20)
project(FluxCompiler VERSION 0.1.0 LANGUAGES C CXX)

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
option(FLUX_ENABLE_TESTS "Build unit tests" ON)
if(FLUX_ENABLE_TESTS)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(INSTALL_GMOCK OFF CACHE BOOL "" FORCE)
endif()
option(FLUX_ENABLE_LIT "Enable LLVM lit tests" ON)
option(FLUX_ENABLE_FUZZING "Enable LLVM LibFuzzer" OFF)

if(FLUX_ENABLE_FUZZING)
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        message(FATAL_ERROR "LibFuzzer requires Clang. Please set CMAKE_CXX_COMPILER to clang++.")
    endif()
    
    # LibFuzzer on MinGW is experimental/unsupported for some versions.
    # We only apply the flags if we are NOT on Windows or if we are using MSVC-style Clang.
    if(WIN32 AND NOT MSVC)
         message(STATUS "LibFuzzer: Skipping fuzzing targets (not fully supported on MinGW/MSYS2 toolchain).")
         set(FLUX_ENABLE_FUZZING OFF CACHE BOOL "" FORCE)
    else()
        add_compile_options(-fsanitize=fuzzer-no-link)
        add_link_options(-fsanitize=fuzzer)
    endif()
endif()

# ============================================================================
# CMake Module Path
# ============================================================================
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# ============================================================================
# Compiler Flags
# ============================================================================
include(FluxCompilerOptions)

# MinGW/MSYS2 Static Linking (Fix for missing DLLs)
if(MINGW)
    add_link_options("-static-libgcc" "-static-libstdc++" "-static")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
endif()

# ============================================================================
# Find Dependencies
# ============================================================================
# Toolchain detection for Windows
if(WIN32)
    # Detect if we are using an MSVC-compatible toolchain (cl.exe or clang-cl)
    if(MSVC OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC"))
        set(FLUX_USING_MSVC_TOOLCHAIN ON)
    else()
        set(FLUX_USING_MSVC_TOOLCHAIN OFF)
    endif()
endif()

# MSYS2/MinGW Path Discovery
if(WIN32)
    if(FLUX_USING_MSVC_TOOLCHAIN)
        # Prioritize Windows-native LLVM installation paths
        set(_FLUX_NATIVE_LLVM_ROOTS
            "${CMAKE_SOURCE_DIR}/llvm-dev"
            "C:/Program Files/LLVM"
            "C:/Program Files (x86)/LLVM"
        )
        
        # If we have an incompatible LLVM already in cache, clear it to allow re-discovery
        if(DEFINED LLVM_DIR AND LLVM_DIR MATCHES "/(clang64|mingw64|ucrt64|msys64)/")
            message(STATUS "Clearing incompatible MSYS2 LLVM from cache to prevent toolchain mismatch...")
            unset(LLVM_DIR CACHE)
        endif()

        # Prepend native paths to search list
        list(INSERT CMAKE_PREFIX_PATH 0 ${_FLUX_NATIVE_LLVM_ROOTS})
    else()
        # Only add MSYS2 paths if we are logically in an MSYS2 environment
        # or if the user hasn't provided a specific LLVM_DIR
        if(DEFINED ENV{MSYSTEM} OR DEFINED ENV{MSYSTEM_PREFIX})
            if(DEFINED ENV{MSYSTEM_PREFIX})
                set(CMAKE_PREFIX_PATH $ENV{MSYSTEM_PREFIX} ${CMAKE_PREFIX_PATH})
            endif()
            list(APPEND CMAKE_PREFIX_PATH 
                "C:/msys64/clang64"
                "C:/msys64/mingw64"
                "C:/msys64/ucrt64"
            )
        endif()
    endif()
endif()

find_package(LLVM REQUIRED CONFIG)

# Toolchain Consistency Check
if(WIN32)
    # Detect mismatch between LLVM library and compiler target
    string(TOLOWER "${LLVM_DIR}" _llvm_dir_lower)
    if(FLUX_USING_MSVC_TOOLCHAIN AND _llvm_dir_lower MATCHES "/(clang64|mingw64|ucrt64|msys64)/")
         message(FATAL_ERROR "Toolchain mismatch: Using MSVC-compatible toolchain with MSYS2/MinGW LLVM library.
                              LLVM found at: ${LLVM_DIR}
                              Please ensure you are NOT mixing MSYS2 environments with standard Windows LLVM/Clang.
                              If using VS Code, check your selected 'Kit' or 'Preset'.")
    endif()

    if(NOT FLUX_USING_MSVC_TOOLCHAIN AND NOT _llvm_dir_lower MATCHES "/(clang64|mingw64|ucrt64|msys64)/")
         # We are trying to use a MinGW/MSYS2 toolchain but found a non-MSYS2 LLVM
         message(STATUS "Toolchain Check: Using a MinGW/MSYS2 compiler with a non-MSYS2 LLVM (at ${LLVM_DIR}). This is often fine, but ensure the LLVM build is ABI-compatible.")
    endif()
endif()

# Resolve LibXml2 warning (often needed by LLVM's Windows builds)
find_package(LibXml2 QUIET)
if(NOT LIBXML2_FOUND)
    message(STATUS "LibXml2 not found. This is optional but may be needed for some LLVM features.")
endif()

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "LLVM include dirs: ${LLVM_INCLUDE_DIRS}")
message(STATUS "LLVM libraries dir: ${LLVM_LIBRARY_DIRS}")
message(STATUS "LLVM definitions: ${LLVM_DEFINITIONS}")

# Treat LLVM headers as system headers to suppress warnings from them
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
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

# ============================================================================
# Packaging (CPack)
# ============================================================================
set(CPACK_PACKAGE_VENDOR "Otabek Sadiridinov")
set(CPACK_PACKAGE_CONTACT "Otabek Sadiridinov")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "The Flux Compiler")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")

set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_DESCRIPTION "Flux is a modern, statically typed programming language for systems programming.")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/otabekoff/flux")

# Architecture detection for package naming
if(NOT FLUX_ARCH)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "amd64|x86_64|AMD64")
        set(FLUX_ARCH "x86_64")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64|ARM64")
        set(FLUX_ARCH "arm64")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i.86|x86")
        set(FLUX_ARCH "x86")
    else()
        set(FLUX_ARCH ${CMAKE_SYSTEM_PROCESSOR})
    endif()
endif()
set(FLUX_ARCH ${FLUX_ARCH} CACHE STRING "Target architecture for packaging")

if(NOT FLUX_OS)
    if(WIN32)
        set(FLUX_OS "windows")
    elseif(APPLE)
        set(FLUX_OS "macos")
    else()
        set(FLUX_OS "linux")
    endif()
endif()
set(FLUX_OS ${FLUX_OS} CACHE STRING "Target OS for packaging")

# --- Packaging (CPack) ---
set(CPACK_PACKAGE_NAME "flux")
set(CPACK_PACKAGE_VENDOR "Flux Project")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "The Flux Programming Language Compiler")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
if(WIN32)
    # WiX requires a license file with a .txt or .rtf extension
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/LICENSE" "${CMAKE_CURRENT_BINARY_DIR}/LICENSE.txt" COPYONLY)
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_BINARY_DIR}/LICENSE.txt")
endif()
set(CPACK_PACKAGE_CONTACT "sadiridinovotabek@gmail.com")

set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${PROJECT_VERSION}-${FLUX_OS}-${FLUX_ARCH}")

# Windows Specific (NSIS & MSI/WiX)
if(WIN32)
    set(CPACK_GENERATOR "ZIP;NSIS;WIX")
    set(CPACK_NSIS_PACKAGE_NAME "Flux Compiler")
    set(CPACK_NSIS_DISPLAY_NAME "Flux Compiler v${PROJECT_VERSION}")
    set(CPACK_NSIS_HELP_LINK "https://github.com/otabekoff/flux")
    set(CPACK_WIX_UPGRADE_GUID "af10e180-b9d1-4e65-83e0-834e404e0960")
    set(CPACK_NSIS_MODIFY_PATH ON)
    set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/assets/logo128x128.ico")
    set(CPACK_NSIS_MUI_UNIICON "${CMAKE_CURRENT_SOURCE_DIR}/assets/logo128x128.ico")
    
    # NSIS Shortcut logic
    set(CPACK_NSIS_MENU_LINKS "bin/flux.exe" "Flux Compiler")
    set(CPACK_NSIS_CREATE_ICONS_EXTRA "
        CreateShortCut '$DESKTOP/Flux Compiler.lnk' '$INSTDIR/bin/flux.exe' '' '$INSTDIR/assets/logo128x128.ico'
    ")
    set(CPACK_NSIS_DELETE_ICONS_EXTRA "
        Delete '$DESKTOP/Flux Compiler.lnk'
    ")
    install(DIRECTORY assets DESTINATION .)
endif()

# Linux Specific (DEB & RPM)
if(UNIX AND NOT APPLE)
    set(CPACK_GENERATOR "TGZ;DEB;RPM")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Otabek <sadiridinovotabek@gmail.com>")
    set(CPACK_DEBIAN_PACKAGE_SECTION "devel")
    set(CPACK_RPM_PACKAGE_LICENSE "Apache-2.0")
    set(CPACK_RPM_PACKAGE_GROUP "Development/Languages")
    set(CPACK_RPM_PACKAGE_RELEASE 1)
endif()

# macOS Specific (DMG)
if(APPLE)
    set(CPACK_GENERATOR "TGZ;DragNDrop")
    set(CPACK_DMG_VOLUME_NAME "Flux")
    set(CPACK_DMG_FORMAT "UDZO")
endif()

include(CPack)
