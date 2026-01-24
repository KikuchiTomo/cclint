#include "rules/rule_base.hpp"

namespace cclint {
namespace rules {

void RuleBase::report_diagnostic(
    diagnostic::DiagnosticEngine& diag_engine,
    const std::string& file_path,
    int line,
    int column,
    const std::string& message,
    const std::vector<diagnostic::FixItHint>& fix_hints) const {

    diagnostic::Diagnostic diag;
    diag.severity = static_cast<diagnostic::Severity>(severity_);
    diag.rule_name = name();
    diag.message = message;
    diag.location.filename = file_path;
    diag.location.line = line;
    diag.location.column = column;
    diag.fix_hints = fix_hints;

    diag_engine.add_diagnostic(diag);
}

std::string RuleBase::severity_string() const {
    switch (severity_) {
    case config::Severity::Error:
        return "error";
    case config::Severity::Warning:
        return "warning";
    case config::Severity::Info:
        return "info";
    default:
        return "unknown";
    }
}

} // namespace rules
} // namespace cclint
