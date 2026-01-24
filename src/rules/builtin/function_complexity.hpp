#pragma once

#include "rules/rule_base.hpp"

namespace cclint {
namespace rules {
namespace builtin {

/// 関数の複雑度チェックルール
/// サイクロマティック複雑度が閾値を超える関数を検出
class FunctionComplexityRule : public RuleBase {
public:
    FunctionComplexityRule();

    std::string name() const override {
        return "function-complexity";
    }

    std::string description() const override {
        return "Check cyclomatic complexity of functions";
    }

    std::string category() const override {
        return "readability";
    }

    void initialize(const RuleParameters& params) override;

    void check_file(const std::string& file_path,
                   const std::string& content,
                   diagnostic::DiagnosticEngine& diag_engine) override;

private:
    int max_complexity_ = 10;  // デフォルト閾値

    /// 関数の複雑度を計算
    int calculate_complexity(const std::string& function_body) const;

    /// 制御文を数える
    int count_control_statements(const std::string& code) const;
};

} // namespace builtin
} // namespace rules
} // namespace cclint
