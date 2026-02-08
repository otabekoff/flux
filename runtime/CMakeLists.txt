add_library(FluxRuntime STATIC
    flux_runtime.cpp
)
set_target_properties(FluxRuntime PROPERTIES 
    OUTPUT_NAME "FluxRuntime"
    PREFIX "lib"
    SUFFIX ".a"
)
target_include_directories(FluxRuntime PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_compile_definitions(FluxRuntime PRIVATE FLUX_RUNTIME_EXPORTS)

# The runtime is plain C++ — no LLVM dependency
include(FluxCompilerOptions)
flux_set_target_options(FluxRuntime)

install(TARGETS FluxRuntime DESTINATION lib)
install(FILES flux_runtime.h DESTINATION include/flux)
