#include "xml_formatter.hpp"

namespace output {

void XmlFormatter::write_header(std::ostream& out) {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    out << "<cclint>\n";
}

void XmlFormatter::format(
    const std::vector<diagnostic::Diagnostic>& diagnostics,
    std::ostream& out) {
    out << indent(1) << "<diagnostics>\n";

    for (const auto& diag : diagnostics) {
        format_diagnostic(diag, out, 2);
    }

    out << indent(1) << "</diagnostics>\n";
}

void XmlFormatter::write_footer(
    const std::vector<diagnostic::Diagnostic>& diagnostics,
    std::ostream& out) {
    auto stats = calculate_statistics(diagnostics);

    out << indent(1) << "<summary>\n";
    out << indent(2) << "<total>" << stats.total_count << "</total>\n";
    out << indent(2) << "<errors>" << stats.error_count << "</errors>\n";
    out << indent(2) << "<warnings>" << stats.warning_count << "</warnings>\n";
    out << indent(2) << "<info>" << stats.info_count << "</info>\n";
    out << indent(1) << "</summary>\n";

    out << "</cclint>\n";
}

void XmlFormatter::format_diagnostic(const diagnostic::Diagnostic& diag,
                                    std::ostream& out,
                                    int indent_level) {
    out << indent(indent_level) << "<diagnostic>\n";

    // 重要度
    out << indent(indent_level + 1) << "<severity>"
        << escape_xml_string(diagnostic::Diagnostic::severity_to_string(diag.severity))
        << "</severity>\n";

    // ルール名
    out << indent(indent_level + 1) << "<rule>"
        << escape_xml_string(diag.rule_name) << "</rule>\n";

    // メッセージ
    out << indent(indent_level + 1) << "<message>"
        << escape_xml_string(diag.message) << "</message>\n";

    // 位置情報
    if (diag.location.is_valid()) {
        format_location(diag.location, out, indent_level + 1);
    }

    // 範囲情報
    if (!diag.ranges.empty()) {
        out << indent(indent_level + 1) << "<ranges>\n";
        for (const auto& range : diag.ranges) {
            format_range(range, out, indent_level + 2);
        }
        out << indent(indent_level + 1) << "</ranges>\n";
    }

    // Fix-itヒント
    if (!diag.fix_hints.empty()) {
        out << indent(indent_level + 1) << "<fixits>\n";
        for (const auto& hint : diag.fix_hints) {
            format_fix_hint(hint, out, indent_level + 2);
        }
        out << indent(indent_level + 1) << "</fixits>\n";
    }

    // 補足情報
    if (!diag.notes.empty()) {
        out << indent(indent_level + 1) << "<notes>\n";
        for (const auto& note : diag.notes) {
            format_diagnostic(note, out, indent_level + 2);
        }
        out << indent(indent_level + 1) << "</notes>\n";
    }

    out << indent(indent_level) << "</diagnostic>\n";
}

void XmlFormatter::format_location(const diagnostic::SourceLocation& loc,
                                  std::ostream& out,
                                  int indent_level) {
    out << indent(indent_level) << "<location>\n";
    out << indent(indent_level + 1) << "<file>"
        << escape_xml_string(loc.filename) << "</file>\n";
    out << indent(indent_level + 1) << "<line>" << loc.line << "</line>\n";
    out << indent(indent_level + 1) << "<column>" << loc.column << "</column>\n";
    out << indent(indent_level) << "</location>\n";
}

void XmlFormatter::format_range(const diagnostic::SourceRange& range,
                               std::ostream& out,
                               int indent_level) {
    out << indent(indent_level) << "<range>\n";
    out << indent(indent_level + 1) << "<begin>\n";
    format_location(range.begin, out, indent_level + 2);
    out << indent(indent_level + 1) << "</begin>\n";
    out << indent(indent_level + 1) << "<end>\n";
    format_location(range.end, out, indent_level + 2);
    out << indent(indent_level + 1) << "</end>\n";
    out << indent(indent_level) << "</range>\n";
}

void XmlFormatter::format_fix_hint(const diagnostic::FixItHint& hint,
                                  std::ostream& out,
                                  int indent_level) {
    out << indent(indent_level) << "<fixit>\n";
    format_range(hint.range, out, indent_level + 1);
    out << indent(indent_level + 1) << "<replacement>"
        << escape_xml_string(hint.replacement_text) << "</replacement>\n";
    out << indent(indent_level) << "</fixit>\n";
}

std::string XmlFormatter::escape_xml_string(const std::string& str) const {
    std::string result;
    for (char c : str) {
        switch (c) {
            case '<':
                result += "&lt;";
                break;
            case '>':
                result += "&gt;";
                break;
            case '&':
                result += "&amp;";
                break;
            case '"':
                result += "&quot;";
                break;
            case '\'':
                result += "&apos;";
                break;
            default:
                result += c;
                break;
        }
    }
    return result;
}

std::string XmlFormatter::indent(int level) const {
    return std::string(level * 2, ' ');
}

}  // namespace output
