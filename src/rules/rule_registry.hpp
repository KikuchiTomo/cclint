#pragma once

#include "rules/rule_base.hpp"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace cclint {
namespace rules {

/// ルールレジストリ（シングルトン）
/// すべての登録されたルールを管理する
class RuleRegistry {
public:
    /// シングルトンインスタンスを取得
    static RuleRegistry& instance();

    /// ルールを登録
    /// @param rule 登録するルール（unique_ptr）
    void register_rule(std::unique_ptr<RuleBase> rule);

    /// ルール名でルールを取得
    /// @param name ルール名
    /// @return ルールのポインタ（存在しない場合はnullptr）
    RuleBase* get_rule(const std::string& name) const;

    /// すべてのルール名を取得
    /// @return ルール名のリスト
    std::vector<std::string> get_all_rule_names() const;

    /// 有効なルールのみを取得
    /// @return 有効なルールのリスト
    std::vector<RuleBase*> get_enabled_rules() const;

    /// カテゴリでルールをフィルタ
    /// @param category カテゴリ名
    /// @return 指定されたカテゴリのルールのリスト
    std::vector<RuleBase*> get_rules_by_category(
        const std::string& category) const;

    /// すべてのルールをクリア（テスト用）
    void clear();

    /// 登録されているルールの数を取得
    size_t size() const { return rules_.size(); }

private:
    RuleRegistry() = default;
    ~RuleRegistry() = default;

    // コピー禁止
    RuleRegistry(const RuleRegistry&) = delete;
    RuleRegistry& operator=(const RuleRegistry&) = delete;

    std::unordered_map<std::string, std::unique_ptr<RuleBase>> rules_;
};

} // namespace rules
} // namespace cclint
