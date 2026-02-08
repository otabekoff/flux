add_executable(flux main.cpp)
target_include_directories(flux PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)
target_link_libraries(flux PRIVATE
    FluxCodeGen
    FluxSema
    FluxParser
    FluxLexer
    FluxAST
    FluxCommon
    ${FLUX_LLVM_LIBS}
)
include(FluxCompilerOptions)
flux_set_target_options(flux)

# Install the compiler binary
install(TARGETS flux DESTINATION bin)
