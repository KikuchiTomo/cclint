#pragma once

#include "rules/rule_base.hpp"

#include <regex>

namespace cclint {
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

    void check_file(const std::string& file_path,
                    const std::string& content,
                    diagnostic::DiagnosticEngine& diag_engine) override;

private:
    // チェック対象の命名パターン
    std::regex function_pattern_;
    std::regex class_pattern_;
    std::regex variable_pattern_;
    std::regex constant_pattern_;

    bool check_functions_ = true;
    bool check_classes_ = true;
    bool check_variables_ = true;
    bool check_constants_ = true;

    void check_function_names(const std::string& file_path,
                              const std::string& content,
                              diagnostic::DiagnosticEngine& diag_engine);

    void check_class_names(const std::string& file_path,
                           const std::string& content,
                           diagnostic::DiagnosticEngine& diag_engine);
};

} // namespace builtin
} // namespace rules
} // namespace cclint
