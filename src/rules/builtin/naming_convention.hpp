#pragma once

#include <memory>
#include <regex>

#include "rules/rule_base.hpp"

namespace cclint {

namespace parser {
class ASTNode;
class TranslationUnitNode;
class ClassNode;
}  // namespace parser

namespace rules {
namespace builtin {

/// 命名規則チェックルール
/// 関数名、変数名、クラス名などの命名規則をチェックする
class NamingConventionRule : public RuleBase {
public:
    NamingConventionRule();

    std::string name() const override { return "naming-convention"; }
    std::string description() const override {
        return "Check naming conventions for functions, variables, and classes";
    }
    std::string category() const override { return "naming"; }

    void initialize(const RuleParameters& params) override;

    void check_file(const std::string& file_path, const std::string& content,
                    diagnostic::DiagnosticEngine& diag_engine) override;

    // AST解析を使ったチェック（アクセス指定子対応）
    void check_ast(const std::string& file_path, std::shared_ptr<parser::TranslationUnitNode> ast,
                   diagnostic::DiagnosticEngine& diag_engine) override;

private:
    // チェック対象の命名パターン
    std::regex function_pattern_;
    std::regex class_pattern_;
    std::regex variable_pattern_;
    std::regex constant_pattern_;

    // アクセス指定子ごとのメソッド命名パターン
    std::regex public_method_pattern_;
    std::regex protected_method_pattern_;
    std::regex private_method_pattern_;

    bool check_functions_ = true;
    bool check_classes_ = true;
    bool check_variables_ = true;
    bool check_constants_ = true;
    bool check_method_access_ = false;  // アクセス指定子ごとのチェックを有効化

    void check_function_names(const std::string& file_path, const std::string& content,
                              diagnostic::DiagnosticEngine& diag_engine);

    void check_class_names(const std::string& file_path, const std::string& content,
                           diagnostic::DiagnosticEngine& diag_engine);

    void check_variable_names(const std::string& file_path, const std::string& content,
                              diagnostic::DiagnosticEngine& diag_engine);

    void check_constant_names(const std::string& file_path, const std::string& content,
                              diagnostic::DiagnosticEngine& diag_engine);

    // ASTベースのクラスメソッドチェック
    void check_class_methods(const std::string& file_path,
                             std::shared_ptr<parser::ClassNode> class_node,
                             diagnostic::DiagnosticEngine& diag_engine);

    void check_ast_recursive(const std::string& file_path, std::shared_ptr<parser::ASTNode> node,
                             diagnostic::DiagnosticEngine& diag_engine);
};

}  // namespace builtin
}  // namespace rules
}  // namespace cclint
