#pragma once

#include <string>
#include <vector>

namespace diagnostic {

// 診断メッセージの重要度
enum class Severity { Error, Warning, Info, Note };

// ソースコードの位置
struct SourceLocation {
    std::string filename;
    int line = 0;
    int column = 0;

    bool is_valid() const { return !filename.empty() && line > 0 && column >= 0; }

    std::string to_string() const;
};

// ソースコードの範囲
struct SourceRange {
    SourceLocation begin;
    SourceLocation end;

    bool is_valid() const { return begin.is_valid() && end.is_valid(); }

    std::string to_string() const;
};

// フィックス提案
struct FixItHint {
    SourceRange range;
    std::string replacement_text;

    std::string to_string() const;
};

// 診断メッセージ
struct Diagnostic {
    Severity severity = Severity::Warning;
    std::string rule_name;
    std::string message;
    SourceLocation location;
    std::vector<SourceRange> ranges;
    std::vector<FixItHint> fix_hints;
    std::vector<Diagnostic> notes;  // 補足情報

    // 診断メッセージを文字列化
    std::string to_string() const;

    // 重要度を文字列化
    static std::string severity_to_string(Severity severity);

    // 文字列から重要度へ変換
    static Severity string_to_severity(const std::string& severity_str);
};

// 診断メッセージのコレクション
class DiagnosticEngine {
public:
    DiagnosticEngine() = default;

    // 診断メッセージを追加
    void add_diagnostic(const Diagnostic& diag);

    // エラーを追加（簡易API）
    void add_error(const std::string& rule_name, const std::string& message,
                   const SourceLocation& location);

    // 警告を追加（簡易API）
    void add_warning(const std::string& rule_name, const std::string& message,
                     const SourceLocation& location);

    // 情報を追加（簡易API）
    void add_info(const std::string& rule_name, const std::string& message,
                  const SourceLocation& location);

    // Fix-itヒント付きの診断を追加
    void add_diagnostic_with_fixit(Severity severity, const std::string& rule_name,
                                   const std::string& message, const SourceLocation& location,
                                   const std::vector<FixItHint>& fix_hints);

    // 全ての診断メッセージを取得
    const std::vector<Diagnostic>& get_diagnostics() const { return diagnostics_; }

    // 診断メッセージをクリア
    void clear() { diagnostics_.clear(); }

    // 統計情報
    size_t error_count() const;
    size_t warning_count() const;
    size_t info_count() const;
    size_t total_count() const { return diagnostics_.size(); }

    // エラーがあるか
    bool has_errors() const { return error_count() > 0; }

private:
    std::vector<Diagnostic> diagnostics_;
};

}  // namespace diagnostic
