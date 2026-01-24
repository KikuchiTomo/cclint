#pragma once

#include "formatter.hpp"

namespace output {

// テキスト形式の出力フォーマッタ
class TextFormatter : public Formatter {
public:
    TextFormatter() = default;

    void format(const std::vector<diagnostic::Diagnostic>& diagnostics, std::ostream& out) override;

    void write_footer(const std::vector<diagnostic::Diagnostic>& diagnostics,
                      std::ostream& out) override;

private:
    // 診断メッセージを1件フォーマット
    void format_diagnostic(const diagnostic::Diagnostic& diag, std::ostream& out);

    // 重要度を色付きで出力
    std::string colorize_severity(diagnostic::Severity severity) const;
};

}  // namespace output
