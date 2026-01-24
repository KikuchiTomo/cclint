#pragma once

#include "formatter.hpp"

namespace output {

// JSON形式の出力フォーマッタ
class JsonFormatter : public Formatter {
public:
    JsonFormatter() = default;

    void format(const std::vector<diagnostic::Diagnostic>& diagnostics, std::ostream& out) override;

private:
    // 診断メッセージを1件JSONに変換
    void format_diagnostic(const diagnostic::Diagnostic& diag, std::ostream& out, bool is_last);

    // SourceLocationをJSONに変換
    void format_location(const diagnostic::SourceLocation& loc, std::ostream& out);

    // SourceRangeをJSONに変換
    void format_range(const diagnostic::SourceRange& range, std::ostream& out);

    // FixItHintをJSONに変換
    void format_fix_hint(const diagnostic::FixItHint& hint, std::ostream& out, bool is_last);

    // 文字列をJSONエスケープ
    std::string escape_json_string(const std::string& str) const;
};

}  // namespace output
