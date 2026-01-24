#include "text_formatter.hpp"

#include <iostream>

namespace output {

void TextFormatter::format(
    const std::vector<diagnostic::Diagnostic>& diagnostics,
    std::ostream& out) {
    for (const auto& diag : diagnostics) {
        format_diagnostic(diag, out);
    }
}

void TextFormatter::format_diagnostic(const diagnostic::Diagnostic& diag,
                                     std::ostream& out) {
    // 位置情報
    if (diag.location.is_valid()) {
        out << diag.location.to_string() << ": ";
    }

    // 重要度（色付き）
    out << colorize_severity(diag.severity);

    // ルール名
    if (!diag.rule_name.empty()) {
        out << " [" << diag.rule_name << "]";
    }

    // メッセージ
    out << ": " << diag.message << "\n";

    // Fix-itヒント（修正提案を表示）
    if (!diag.fix_hints.empty()) {
        const char* green = "\033[1;32m";
        const char* reset = "\033[0m";

        for (const auto& hint : diag.fix_hints) {
            out << "  " << green << "fix-it:" << reset << " "
                << hint.range.to_string() << "\n";
            if (!hint.replacement_text.empty()) {
                out << "    " << green << "replace with:" << reset
                    << " '" << hint.replacement_text << "'\n";
            }
        }
    }

    // 補足情報（ノート）
    for (const auto& note : diag.notes) {
        out << "  ";
        format_diagnostic(note, out);
    }
}

void TextFormatter::write_footer(
    const std::vector<diagnostic::Diagnostic>& diagnostics,
    std::ostream& out) {
    auto stats = calculate_statistics(diagnostics);

    if (stats.total_count == 0) {
        out << "No issues found.\n";
        return;
    }

    out << "\n";
    out << stats.error_count << " error(s), "
        << stats.warning_count << " warning(s), "
        << stats.info_count << " info message(s) generated.\n";
}

std::string TextFormatter::colorize_severity(
    diagnostic::Severity severity) const {
    // ANSI color codes（環境変数でカラー出力を制御可能）
    const char* reset = "\033[0m";
    const char* red = "\033[1;31m";
    const char* yellow = "\033[1;33m";
    const char* cyan = "\033[1;36m";
    const char* white = "\033[1;37m";

    // TODO: 環境変数やターミナル検出でカラー出力を制御
    // 現在は常にカラー出力を使用
    bool use_color = true;

    if (!use_color) {
        return diagnostic::Diagnostic::severity_to_string(severity);
    }

    std::string result;
    switch (severity) {
        case diagnostic::Severity::Error:
            result = std::string(red) + "error" + reset;
            break;
        case diagnostic::Severity::Warning:
            result = std::string(yellow) + "warning" + reset;
            break;
        case diagnostic::Severity::Info:
            result = std::string(cyan) + "info" + reset;
            break;
        case diagnostic::Severity::Note:
            result = std::string(white) + "note" + reset;
            break;
    }

    return result;
}

}  // namespace output
