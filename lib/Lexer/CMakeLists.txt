add_library(FluxLexer STATIC
    Lexer.cpp
)

target_link_libraries(FluxLexer PUBLIC FluxCommon)

flux_set_target_options(FluxLexer)
