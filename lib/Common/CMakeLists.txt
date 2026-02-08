add_library(FluxCommon STATIC
    SourceLocation.cpp
    Diagnostics.cpp
)

target_include_directories(FluxCommon PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

flux_set_target_options(FluxCommon)
