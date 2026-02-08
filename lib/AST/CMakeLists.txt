add_library(FluxAST STATIC
    ASTVisitor.cpp
    ASTPrinter.cpp
)

target_link_libraries(FluxAST PUBLIC FluxCommon)

flux_set_target_options(FluxAST)
