#include "flux/Sema/Sema.h"

namespace flux {

Sema::Sema(DiagnosticEngine &diag) : diag_(diag), globalScope_("global") {}

bool Sema::analyze(ast::Module &module) {
  size_t errorsBefore = diag_.getErrorCount();

  // Phase 1: Name resolution
  NameResolver resolver(diag_, globalScope_);
  resolver.resolve(module);

  if (diag_.getErrorCount() > errorsBefore) {
    return false; // Name resolution errors prevent type checking
  }

  // Phase 2: Type checking
  TypeChecker checker(diag_, globalScope_);
  checker.check(module);

  return diag_.getErrorCount() == errorsBefore;
}

} // namespace flux