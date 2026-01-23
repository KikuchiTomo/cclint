#pragma once

#include "formatter.hpp"

namespace output {

// XML形式の出力フォーマッタ
class XmlFormatter : public Formatter {
public:
    XmlFormatter() = default;

    void format(const std::vector<diagnostic::Diagnostic>& diagnostics,
               std::ostream& out) override;

    void write_header(std::ostream& out) override;

    void write_footer(const std::vector<diagnostic::Diagnostic>& diagnostics,
                     std::ostream& out) override;

private:
    // 診断メッセージを1件XMLに変換
    void format_diagnostic(const diagnostic::Diagnostic& diag,
                          std::ostream& out,
                          int indent_level = 1);

    // SourceLocationをXMLに変換
    void format_location(const diagnostic::SourceLocation& loc,
                        std::ostream& out,
                        int indent_level);

    // SourceRangeをXMLに変換
    void format_range(const diagnostic::SourceRange& range,
                     std::ostream& out,
                     int indent_level);

    // FixItHintをXMLに変換
    void format_fix_hint(const diagnostic::FixItHint& hint,
                        std::ostream& out,
                        int indent_level);

    // 文字列をXMLエスケープ
    std::string escape_xml_string(const std::string& str) const;

    // インデント文字列を生成
    std::string indent(int level) const;
};

}  // namespace output
