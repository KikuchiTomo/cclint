#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <vector>

#include "diagnostic/diagnostic.hpp"
#include "rules/rule_base.hpp"

namespace clang {
class ASTUnit;
}

namespace cclint {

namespace parser {
class TranslationUnitNode;
}

namespace rules {

/// ルール実行の統計情報
struct RuleExecutionStats {
    std::string rule_name;
    std::chrono::milliseconds execution_time{0};
    size_t diagnostics_count = 0;
    bool timed_out = false;
    bool failed = false;
    std::string error_message;
};

/// ルールエグゼキューター
/// 登録されたルールを実行し、診断を収集する
class RuleExecutor {
public:
    /// コンストラクタ
    /// @param timeout_ms ルールごとのタイムアウト時間（ミリ秒、0で無制限）
    explicit RuleExecutor(int timeout_ms = 0);

    /// ファイルに対してすべての有効なルールを実行（テキストベース）
    /// @param file_path ファイルパス
    /// @param content ファイル内容
    /// @param diag_engine 診断エンジン
    /// @return 実行統計のリスト
    std::vector<RuleExecutionStats> execute_text_rules(const std::string& file_path,
                                                       const std::string& content,
                                                       diagnostic::DiagnosticEngine& diag_engine);

    /// ASTに対してすべての有効なルールを実行（独自AST）
    /// @param file_path ファイルパス
    /// @param ast AST
    /// @param diag_engine 診断エンジン
    /// @return 実行統計のリスト
    std::vector<RuleExecutionStats>
    execute_ast_rules(const std::string& file_path,
                      std::shared_ptr<parser::TranslationUnitNode> ast,
                      diagnostic::DiagnosticEngine& diag_engine);

    /// ASTに対してすべての有効なルールを実行（Clang AST）
    /// @param ast_unit ASTUnit
    /// @param diag_engine 診断エンジン
    /// @return 実行統計のリスト
    std::vector<RuleExecutionStats>
    execute_clang_ast_rules(clang::ASTUnit* ast_unit, diagnostic::DiagnosticEngine& diag_engine);

    /// 特定のルールのみを実行（テキストベース）
    /// @param rule_name ルール名
    /// @param file_path ファイルパス
    /// @param content ファイル内容
    /// @param diag_engine 診断エンジン
    /// @return 実行統計
    RuleExecutionStats execute_text_rule(const std::string& rule_name, const std::string& file_path,
                                         const std::string& content,
                                         diagnostic::DiagnosticEngine& diag_engine);

    /// 特定のルールのみを実行（AST）
    /// @param rule_name ルール名
    /// @param ast_unit ASTUnit
    /// @param diag_engine 診断エンジン
    /// @return 実行統計
    RuleExecutionStats execute_ast_rule(const std::string& rule_name, clang::ASTUnit* ast_unit,
                                        diagnostic::DiagnosticEngine& diag_engine);

    /// タイムアウト時間を設定
    void set_timeout(int timeout_ms) { timeout_ms_ = timeout_ms; }

    /// タイムアウト時間を取得
    int get_timeout() const { return timeout_ms_; }

private:
    /// ルールを実行し、統計を収集するヘルパー
    template <typename Func>
    RuleExecutionStats execute_with_stats(RuleBase* rule, diagnostic::DiagnosticEngine& diag_engine,
                                          Func&& func);

    int timeout_ms_;
};

}  // namespace rules
}  // namespace cclint
