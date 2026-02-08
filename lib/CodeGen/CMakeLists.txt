add_library(FluxCodeGen STATIC
    TypeMapper.cpp
    IREmitter.cpp
    CodeGen.cpp
)
target_include_directories(FluxCodeGen PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)
target_link_libraries(FluxCodeGen PUBLIC
    FluxAST
    FluxCommon
    ${FLUX_LLVM_LIBS}
)
include(FluxCompilerOptions)
flux_set_target_options(FluxCodeGen)
