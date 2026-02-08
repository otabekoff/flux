include(FetchContent)

# --------------------------------------------------------------------------
# GoogleTest
# --------------------------------------------------------------------------
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.15.2
)
# Prevent GTest from overriding our compiler settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Suppress warnings from GoogleTest headers (third-party code)
get_target_property(_gtest_inc gtest INTERFACE_INCLUDE_DIRECTORIES)
set_target_properties(gtest PROPERTIES
    INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${_gtest_inc}")
set_target_properties(gtest_main PROPERTIES
    INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${_gtest_inc}")
# Suppress warnings in gtest's own compilation
target_compile_options(gtest PRIVATE $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-w>)
target_compile_options(gmock PRIVATE $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-w>)

enable_testing()

# --------------------------------------------------------------------------
# Unit tests
# --------------------------------------------------------------------------
add_executable(flux_unit_tests
    unit/LexerTest.cpp
    unit/ParserTest.cpp
    unit/SemaTest.cpp
)
target_include_directories(flux_unit_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)
target_link_libraries(flux_unit_tests PRIVATE
    FluxSema
    FluxParser
    FluxLexer
    FluxAST
    FluxCommon
    GTest::gtest_main
)
include(FluxCompilerOptions)
flux_set_target_options(flux_unit_tests)

include(GoogleTest)
gtest_discover_tests(flux_unit_tests)
