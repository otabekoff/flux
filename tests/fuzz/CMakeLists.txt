if(FLUX_ENABLE_FUZZING)
    add_executable(flux_lexer_fuzzer LexerFuzzer.cpp)
    target_link_libraries(flux_lexer_fuzzer PRIVATE FluxLexer FluxCommon)
    
    # Ensure LibFuzzer is linked
    set_target_properties(flux_lexer_fuzzer PROPERTIES 
        COMPILE_FLAGS "-fsanitize=fuzzer"
        LINK_FLAGS "-fsanitize=fuzzer"
    )
endif()
