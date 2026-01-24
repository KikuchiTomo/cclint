#pragma once

#include "rules/rule_base.hpp"

namespace cclint {
namespace rules {
namespace builtin {

/// 最大行長チェックルール
/// 各行の長さが指定された最大値を超えていないかをチェック
class MaxLineLengthRule : public RuleBase {
public:
    MaxLineLengthRule();

    std::string name() const override { return "max-line-length"; }
    std::string description() const override {
        return "Check that lines do not exceed maximum length";
    }
    std::string category() const override { return "style"; }

    void initialize(const RuleParameters& params) override;

    void check_file(const std::string& file_path,
                    const std::string& content,
                    diagnostic::DiagnosticEngine& diag_engine) override;

private:
    size_t max_length_ = 80;
    bool ignore_comments_ = false;
    bool ignore_urls_ = true;
};

} // namespace builtin
} // namespace rules
} // namespace cclint
