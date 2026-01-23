#include "json_formatter.hpp"

namespace output {

void JsonFormatter::format(
    const std::vector<diagnostic::Diagnostic>& diagnostics,
    std::ostream& out) {
    out << "{\n";
    out << "  \"diagnostics\": [\n";

    for (size_t i = 0; i < diagnostics.size(); ++i) {
        out << "    ";
        format_diagnostic(diagnostics[i], out, i == diagnostics.size() - 1);
    }

    out << "  ],\n";

    // 統計情報
    auto stats = calculate_statistics(diagnostics);
    out << "  \"summary\": {\n";
    out << "    \"total\": " << stats.total_count << ",\n";
    out << "    \"errors\": " << stats.error_count << ",\n";
    out << "    \"warnings\": " << stats.warning_count << ",\n";
    out << "    \"info\": " << stats.info_count << "\n";
    out << "  }\n";
    out << "}\n";
}

void JsonFormatter::format_diagnostic(const diagnostic::Diagnostic& diag,
                                     std::ostream& out,
                                     bool is_last) {
    out << "{\n";

    // 重要度
    out << "      \"severity\": \""
        << escape_json_string(diagnostic::Diagnostic::severity_to_string(diag.severity))
        << "\",\n";

    // ルール名
    out << "      \"rule\": \"" << escape_json_string(diag.rule_name) << "\",\n";

    // メッセージ
    out << "      \"message\": \"" << escape_json_string(diag.message) << "\"";

    // 位置情報
    if (diag.location.is_valid()) {
        out << ",\n      \"location\": ";
        format_location(diag.location, out);
    }

    // 範囲情報
    if (!diag.ranges.empty()) {
        out << ",\n      \"ranges\": [\n";
        for (size_t i = 0; i < diag.ranges.size(); ++i) {
            out << "        ";
            format_range(diag.ranges[i], out);
            if (i < diag.ranges.size() - 1) {
                out << ",";
            }
            out << "\n";
        }
        out << "      ]";
    }

    // Fix-itヒント
    if (!diag.fix_hints.empty()) {
        out << ",\n      \"fixits\": [\n";
        for (size_t i = 0; i < diag.fix_hints.size(); ++i) {
            out << "        ";
            format_fix_hint(diag.fix_hints[i], out, i == diag.fix_hints.size() - 1);
        }
        out << "      ]";
    }

    // 補足情報
    if (!diag.notes.empty()) {
        out << ",\n      \"notes\": [\n";
        for (size_t i = 0; i < diag.notes.size(); ++i) {
            out << "        ";
            format_diagnostic(diag.notes[i], out, i == diag.notes.size() - 1);
        }
        out << "      ]";
    }

    out << "\n    }";
    if (!is_last) {
        out << ",";
    }
    out << "\n";
}

void JsonFormatter::format_location(const diagnostic::SourceLocation& loc,
                                   std::ostream& out) {
    out << "{\n";
    out << "        \"file\": \"" << escape_json_string(loc.filename) << "\",\n";
    out << "        \"line\": " << loc.line << ",\n";
    out << "        \"column\": " << loc.column << "\n";
    out << "      }";
}

void JsonFormatter::format_range(const diagnostic::SourceRange& range,
                                std::ostream& out) {
    out << "{\n";
    out << "          \"begin\": ";
    format_location(range.begin, out);
    out << ",\n";
    out << "          \"end\": ";
    format_location(range.end, out);
    out << "\n        }";
}

void JsonFormatter::format_fix_hint(const diagnostic::FixItHint& hint,
                                   std::ostream& out,
                                   bool is_last) {
    out << "{\n";
    out << "          \"range\": ";
    format_range(hint.range, out);
    out << ",\n";
    out << "          \"replacement\": \""
        << escape_json_string(hint.replacement_text) << "\"\n";
    out << "        }";
    if (!is_last) {
        out << ",";
    }
    out << "\n";
}

std::string JsonFormatter::escape_json_string(const std::string& str) const {
    std::string result;
    for (char c : str) {
        switch (c) {
            case '"':
                result += "\\\"";
                break;
            case '\\':
                result += "\\\\";
                break;
            case '\b':
                result += "\\b";
                break;
            case '\f':
                result += "\\f";
                break;
            case '\n':
                result += "\\n";
                break;
            case '\r':
                result += "\\r";
                break;
            case '\t':
                result += "\\t";
                break;
            default:
                result += c;
                break;
        }
    }
    return result;
}

}  // namespace output
