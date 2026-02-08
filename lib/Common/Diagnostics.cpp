#include "flux/Common/Diagnostics.h"

#include <iostream>
#include <sstream>

namespace flux {

DiagnosticEngine::DiagnosticEngine() {
  handler_ = [this](const Diagnostic &diag) { defaultHandler(diag); };
}

void DiagnosticEngine::setHandler(DiagnosticHandler handler) {
  handler_ = std::move(handler);
}

void DiagnosticEngine::setSourceManager(const SourceManager *sm) {
  sourceManager_ = sm;
}

void DiagnosticEngine::emitError(SourceLocation loc,
                                 const std::string &message) {
  emit(Diagnostic{DiagnosticSeverity::Error, loc, message, {}, {}});
}

void DiagnosticEngine::emitWarning(SourceLocation loc,
                                   const std::string &message) {
  emit(Diagnostic{DiagnosticSeverity::Warning, loc, message, {}, {}});
}

void DiagnosticEngine::emitNote(SourceLocation loc,
                                const std::string &message) {
  emit(Diagnostic{DiagnosticSeverity::Note, loc, message, {}, {}});
}

void DiagnosticEngine::emitFatal(SourceLocation loc,
                                 const std::string &message) {
  emit(Diagnostic{DiagnosticSeverity::Fatal, loc, message, {}, {}});
}

void DiagnosticEngine::emit(Diagnostic diag) {
  switch (diag.severity) {
  case DiagnosticSeverity::Error:
  case DiagnosticSeverity::Fatal:
    ++errorCount_;
    break;
  case DiagnosticSeverity::Warning:
    ++warningCount_;
    break;
  case DiagnosticSeverity::Note:
    break;
  }

  diagnostics_.push_back(diag);

  if (handler_) {
    handler_(diag);
  }
}

void DiagnosticEngine::reset() {
  diagnostics_.clear();
  errorCount_ = 0;
  warningCount_ = 0;
}

std::string
DiagnosticEngine::severityString(DiagnosticSeverity severity) const {
  switch (severity) {
  case DiagnosticSeverity::Note:
    return "note";
  case DiagnosticSeverity::Warning:
    return "warning";
  case DiagnosticSeverity::Error:
    return "error";
  case DiagnosticSeverity::Fatal:
    return "fatal error";
  }
  return "unknown";
}

std::string DiagnosticEngine::formatDiagnostic(const Diagnostic &diag) const {
  std::ostringstream out;

  // Severity and message
  out << severityString(diag.severity) << ": " << diag.message << "\n";

  // Location
  if (diag.location.isValid()) {
    out << "  --> " << diag.location.toString() << "\n";

    // Show source line if source manager is available
    if (sourceManager_) {
      // Get the source line containing the error
      // For now, we show the basic location. Rich source display
      // will be enhanced when the full source manager integration matures.
      out << "   |\n";
      out << "   |\n";
    }
  }

  // Notes
  for (const auto &note : diag.notes) {
    out << "  note: " << note.message << "\n";
    if (note.location.isValid()) {
      out << "    --> " << note.location.toString() << "\n";
    }
  }

  // Fix suggestions
  for (const auto &fix : diag.fixes) {
    out << "  help: " << fix.description << "\n";
    if (!fix.replacement.empty()) {
      out << "    suggested: " << fix.replacement << "\n";
    }
  }

  return out.str();
}

void DiagnosticEngine::defaultHandler(const Diagnostic &diag) {
  std::string formatted = formatDiagnostic(diag);
  std::cerr << formatted;
}

} // namespace flux
