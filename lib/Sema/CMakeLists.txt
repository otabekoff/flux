add_library(FluxSema STATIC
    NameResolution.cpp
    TypeChecker.cpp
    Sema.cpp
)
target_include_directories(FluxSema PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)
target_link_libraries(FluxSema PUBLIC FluxAST FluxCommon)
include(FluxCompilerOptions)
flux_set_target_options(FluxSema)
