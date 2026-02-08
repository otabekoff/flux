#pragma once

#include "flux/AST/AST.h"
#include "flux/Common/Diagnostics.h"
#include "flux/Sema/NameResolution.h"
#include "flux/Sema/TypeChecker.h"

namespace flux {

/// Top-level semantic analysis driver.
///
/// Orchestrates name resolution and type checking in the correct
/// order over an AST module.
class Sema {
public:
    explicit Sema(DiagnosticEngine& diag);

    /// Run all semantic analysis passes on the module.
    /// Returns true if no errors were found.
    bool analyze(ast::Module& module);

    /// Access the global scope after analysis.
    const Scope& getGlobalScope() const { return globalScope_; }

private:
    DiagnosticEngine& diag_;
    Scope globalScope_;
};

} // namespace flux
