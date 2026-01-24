#pragma once

#include "diagnostic/diagnostic.hpp"
#include "config/config_types.hpp"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace clang {
class ASTUnit;
class ASTContext;
class Decl;
class Stmt;
} // namespace clang

namespace cclint {

namespace parser {
class TranslationUnitNode;
}

namespace rules {

/// ルールのパラメータを表す型
using RuleParameters = std::unordered_map<std::string, std::string>;

/// ルールの基底クラス
/// すべてのルール（ビルトインおよびLua）はこのインターフェースを実装する
class RuleBase {
public:
    virtual ~RuleBase() = default;

    /// ルール名を取得
    virtual std::string name() const = 0;

    /// ルールの説明を取得
    virtual std::string description() const = 0;

    /// ルールのカテゴリを取得（例: "naming", "style", "security"）
    virtual std::string category() const = 0;

    /// ルールの初期化
    /// @param params ルールのパラメータ
    virtual void initialize(const RuleParameters& params) = 0;

    /// ファイル全体に対するチェック（テキストベース）
    /// AST解析が不要な簡易ルール用
    /// @param file_path ファイルパス
    /// @param content ファイルの内容
    /// @param diag_engine 診断エンジン
    virtual void check_file(const std::string& file_path,
                            const std::string& content,
                            diagnostic::DiagnosticEngine& diag_engine) {
        // デフォルトは何もしない（オプショナル）
        (void)file_path;
        (void)content;
        (void)diag_engine;
    }

    /// AST全体に対するチェック (Clang AST)
    /// @param ast_unit ASTUnit
    /// @param diag_engine 診断エンジン
    virtual void check_ast(clang::ASTUnit* ast_unit,
                           diagnostic::DiagnosticEngine& diag_engine) {
        // デフォルトは何もしない（オプショナル）
        (void)ast_unit;
        (void)diag_engine;
    }

    /// AST全体に対するチェック (独自AST)
    /// @param file_path ファイルパス
    /// @param ast 独自AST
    /// @param diag_engine 診断エンジン
    virtual void check_ast(const std::string& file_path,
                           std::shared_ptr<parser::TranslationUnitNode> ast,
                           diagnostic::DiagnosticEngine& diag_engine) {
        // デフォルトは何もしない（オプショナル）
        (void)file_path;
        (void)ast;
        (void)diag_engine;
    }

    /// ルールの有効化/無効化を設定
    void set_enabled(bool enabled) { enabled_ = enabled; }

    /// ルールが有効かどうかを取得
    bool is_enabled() const { return enabled_; }

    /// severity を設定
    void set_severity(config::Severity severity) { severity_ = severity; }

    /// severity を取得
    config::Severity get_severity() const { return severity_; }

protected:
    /// 診断を報告するヘルパー関数
    void report_diagnostic(diagnostic::DiagnosticEngine& diag_engine,
                           const std::string& file_path,
                           int line,
                           int column,
                           const std::string& message,
                           const std::vector<diagnostic::FixItHint>& fix_hints =
                               {}) const;

    /// severity の文字列を取得
    std::string severity_string() const;

private:
    bool enabled_ = true;
    config::Severity severity_ = config::Severity::Warning;
};

} // namespace rules
} // namespace cclint
