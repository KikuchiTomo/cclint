#include "diagnostic.hpp"

#include <sstream>

namespace diagnostic {

std::string SourceLocation::to_string() const {
    if (!is_valid()) {
        return "<invalid location>";
    }

    std::ostringstream oss;
    oss << filename << ":" << line << ":" << column;
    return oss.str();
}

std::string SourceRange::to_string() const {
    if (!is_valid()) {
        return "<invalid range>";
    }

    std::ostringstream oss;
    oss << begin.to_string() << "-" << end.line << ":" << end.column;
    return oss.str();
}

std::string FixItHint::to_string() const {
    std::ostringstream oss;
    oss << "Fix: replace " << range.to_string() << " with \"" << replacement_text << "\"";
    return oss.str();
}

std::string Diagnostic::to_string() const {
    std::ostringstream oss;

    // 重要度と位置
    if (location.is_valid()) {
        oss << location.to_string() << ": ";
    }
    oss << severity_to_string(severity);

    // ルール名
    if (!rule_name.empty()) {
        oss << " [" << rule_name << "]";
    }

    // メッセージ
    oss << ": " << message;

    // Fix-itヒント
    for (const auto& hint : fix_hints) {
        oss << "\n  " << hint.to_string();
    }

    // 補足情報
    for (const auto& note : notes) {
        oss << "\n  " << note.to_string();
    }

    return oss.str();
}

std::string Diagnostic::severity_to_string(Severity severity) {
    switch (severity) {
        case Severity::Error:
            return "error";
        case Severity::Warning:
            return "warning";
        case Severity::Info:
            return "info";
        case Severity::Note:
            return "note";
        default:
            return "unknown";
    }
}

Severity Diagnostic::string_to_severity(const std::string& severity_str) {
    if (severity_str == "error" || severity_str == "Error") {
        return Severity::Error;
    } else if (severity_str == "warning" || severity_str == "Warning") {
        return Severity::Warning;
    } else if (severity_str == "info" || severity_str == "Info") {
        return Severity::Info;
    } else if (severity_str == "note" || severity_str == "Note") {
        return Severity::Note;
    } else {
        return Severity::Warning;  // デフォルト
    }
}

void DiagnosticEngine::add_diagnostic(const Diagnostic& diag) {
    diagnostics_.push_back(diag);
}

void DiagnosticEngine::add_error(const std::string& rule_name, const std::string& message,
                                 const SourceLocation& location) {
    Diagnostic diag;
    diag.severity = Severity::Error;
    diag.rule_name = rule_name;
    diag.message = message;
    diag.location = location;
    add_diagnostic(diag);
}

void DiagnosticEngine::add_warning(const std::string& rule_name, const std::string& message,
                                   const SourceLocation& location) {
    Diagnostic diag;
    diag.severity = Severity::Warning;
    diag.rule_name = rule_name;
    diag.message = message;
    diag.location = location;
    add_diagnostic(diag);
}

void DiagnosticEngine::add_info(const std::string& rule_name, const std::string& message,
                                const SourceLocation& location) {
    Diagnostic diag;
    diag.severity = Severity::Info;
    diag.rule_name = rule_name;
    diag.message = message;
    diag.location = location;
    add_diagnostic(diag);
}

size_t DiagnosticEngine::error_count() const {
    size_t count = 0;
    for (const auto& diag : diagnostics_) {
        if (diag.severity == Severity::Error) {
            ++count;
        }
    }
    return count;
}

size_t DiagnosticEngine::warning_count() const {
    size_t count = 0;
    for (const auto& diag : diagnostics_) {
        if (diag.severity == Severity::Warning) {
            ++count;
        }
    }
    return count;
}

size_t DiagnosticEngine::info_count() const {
    size_t count = 0;
    for (const auto& diag : diagnostics_) {
        if (diag.severity == Severity::Info) {
            ++count;
        }
    }
    return count;
}

void DiagnosticEngine::add_diagnostic_with_fixit(Severity severity, const std::string& rule_name,
                                                   const std::string& message,
                                                   const SourceLocation& location,
                                                   const std::vector<FixItHint>& fix_hints) {
    Diagnostic diag;
    diag.severity = severity;
    diag.rule_name = rule_name;
    diag.message = message;
    diag.location = location;
    diag.fix_hints = fix_hints;
    add_diagnostic(diag);
}

}  // namespace diagnostic
