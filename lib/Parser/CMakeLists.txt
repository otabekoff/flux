add_library(FluxParser STATIC
    Parser.cpp
)

target_link_libraries(FluxParser PUBLIC FluxLexer FluxAST)

flux_set_target_options(FluxParser)
