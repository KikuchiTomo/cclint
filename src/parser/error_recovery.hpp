#pragma once

#include <string>
#include <vector>

#include "parser/token_types_enhanced.hpp"

namespace cclint {
namespace parser {

/// エラーリカバリ戦略
enum class RecoveryStrategy {
    SkipToSemicolon,      // セミコロンまでスキップ
    SkipToCloseBrace,     // 閉じ括弧までスキップ
    SkipToNextStatement,  // 次の文までスキップ
    InsertMissing,        // 欠けているトークンを挿入
    DeleteUnexpected,     // 予期しないトークンを削除
    None                  // リカバリしない
};

/// パースエラー情報
struct ParseError {
    std::string message;
    int line = 0;
    int column = 0;
    std::string filename;
    RecoveryStrategy recovery = RecoveryStrategy::None;

    /// エラーの重要度
    enum class Severity {
        Fatal,    // 致命的エラー（パース続行不可能）
        Error,    // エラー（リカバリ可能）
        Warning,  // 警告
        Note      // 補足情報
    };

    Severity severity = Severity::Error;

    /// 期待されたトークン
    std::vector<TokenType> expected_tokens;

    /// 実際に見つかったトークン
    TokenType found_token = TokenType::Unknown;

    /// 修正提案
    std::string fix_suggestion;

    /// エラーメッセージを生成
    std::string format() const;
};

/// エラーリカバリマネージャー
class ErrorRecovery {
public:
    /// エラーを記録
    void add_error(const ParseError& error);

    /// 全エラーを取得
    const std::vector<ParseError>& errors() const { return errors_; }

    /// エラーがあるか
    bool has_errors() const { return !errors_.empty(); }

    /// 致命的エラーがあるか
    bool has_fatal_errors() const;

    /// エラー数を取得
    size_t error_count() const { return errors_.size(); }

    /// 警告数を取得
    size_t warning_count() const;

    /// エラーをクリア
    void clear() { errors_.clear(); }

    /// 最適なリカバリ戦略を提案
    /// @param expected 期待されたトークン
    /// @param found 実際のトークン
    /// @return 推奨されるリカバリ戦略
    static RecoveryStrategy suggest_recovery(const std::vector<TokenType>& expected,
                                             TokenType found);

    /// エラーメッセージを整形
    /// @param errors エラーリスト
    /// @return 整形されたエラーメッセージ
    static std::string format_errors(const std::vector<ParseError>& errors);

private:
    std::vector<ParseError> errors_;
};

}  // namespace parser
}  // namespace cclint
