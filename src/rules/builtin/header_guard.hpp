#pragma once

#include "rules/rule_base.hpp"

namespace cclint {
namespace rules {
namespace builtin {

/// ヘッダーガードチェックルール
/// #ifndef/#define/#endif または #pragma once の存在をチェック
class HeaderGuardRule : public RuleBase {
public:
    HeaderGuardRule();

    std::string name() const override { return "header-guard"; }
    std::string description() const override {
        return "Check for proper header guards in header files";
    }
    std::string category() const override { return "structure"; }

    void initialize(const RuleParameters& params) override;

    void check_file(const std::string& file_path, const std::string& content,
                    diagnostic::DiagnosticEngine& diag_engine) override;

private:
    bool allow_pragma_once_ = true;
    bool require_pragma_once_ = false;

    bool is_header_file(const std::string& file_path) const;
    bool has_pragma_once(const std::string& content) const;
    bool has_header_guard(const std::string& content) const;
};

}  // namespace builtin
}  // namespace rules
}  // namespace cclint
