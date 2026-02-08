#pragma once

#include "flux/Common/SourceLocation.h"

#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace flux {

/// Severity level for diagnostics.
enum class DiagnosticSeverity : uint8_t {
    Note,
    Warning,
    Error,
    Fatal
};

/// A single diagnostic message with source location and optional hints.
struct Diagnostic {
    DiagnosticSeverity severity;
    SourceLocation location;
    std::string message;

    // Optional: related notes and hints
    struct Note {
        SourceLocation location;
        std::string message;
    };
    std::vector<Note> notes;

    // Optional: suggested fix
    struct Fix {
        SourceRange range;
        std::string replacement;
        std::string description;
    };
    std::vector<Fix> fixes;
};

/// Diagnostic engine for the Flux compiler.
/// Collects and formats error messages in Flux's rich error style.
///
/// Example output:
///   error[E0308]: mismatched types
///     --> src/main.flux:12:9
///      |
///   12 |     let x: Int32 = "hello";
///      |            -----   ^^^^^^^ expected Int32, found String
///      |
///   help: you might need to parse the string to an integer
///
class DiagnosticEngine {
public:
    using DiagnosticHandler = std::function<void(const Diagnostic&)>;

    DiagnosticEngine();

    /// Set a custom handler for diagnostics (default: print to stderr).
    void setHandler(DiagnosticHandler handler);

    /// Set the source manager for rich error formatting.
    void setSourceManager(const SourceManager* sm);

    // --- Emission methods ---

    void emitError(SourceLocation loc, const std::string& message);
    void emitWarning(SourceLocation loc, const std::string& message);
    void emitNote(SourceLocation loc, const std::string& message);
    void emitFatal(SourceLocation loc, const std::string& message);

    /// Emit a diagnostic with full detail.
    void emit(Diagnostic diag);

    // --- Query ---

    bool hasErrors() const { return errorCount_ > 0; }
    uint32_t errorCount() const { return errorCount_; }
    uint32_t warningCount() const { return warningCount_; }
    uint32_t getErrorCount() const { return errorCount_; }
    uint32_t getWarningCount() const { return warningCount_; }

    /// Get all collected diagnostics (when using collection mode).
    const std::vector<Diagnostic>& diagnostics() const { return diagnostics_; }

    /// Reset all state.
    void reset();

private:
    void defaultHandler(const Diagnostic& diag);
    std::string formatDiagnostic(const Diagnostic& diag) const;
    std::string severityString(DiagnosticSeverity severity) const;

    DiagnosticHandler handler_;
    const SourceManager* sourceManager_ = nullptr;
    std::vector<Diagnostic> diagnostics_;
    uint32_t errorCount_ = 0;
    uint32_t warningCount_ = 0;
};

} // namespace flux
