#include "rules/rule_registry.hpp"

namespace cclint {
namespace rules {

RuleRegistry& RuleRegistry::instance() {
    static RuleRegistry registry;
    return registry;
}

void RuleRegistry::register_rule(std::unique_ptr<RuleBase> rule) {
    if (!rule) {
        return;
    }

    const std::string name = rule->name();
    rules_[name] = std::move(rule);
}

RuleBase* RuleRegistry::get_rule(const std::string& name) const {
    auto it = rules_.find(name);
    if (it != rules_.end()) {
        return it->second.get();
    }
    return nullptr;
}

std::vector<std::string> RuleRegistry::get_all_rule_names() const {
    std::vector<std::string> names;
    names.reserve(rules_.size());

    for (const auto& pair : rules_) {
        names.push_back(pair.first);
    }

    return names;
}

std::vector<RuleBase*> RuleRegistry::get_enabled_rules() const {
    std::vector<RuleBase*> enabled_rules;

    for (const auto& pair : rules_) {
        if (pair.second->is_enabled()) {
            enabled_rules.push_back(pair.second.get());
        }
    }

    return enabled_rules;
}

std::vector<RuleBase*> RuleRegistry::get_rules_by_category(
    const std::string& category) const {
    std::vector<RuleBase*> filtered_rules;

    for (const auto& pair : rules_) {
        if (pair.second->category() == category) {
            filtered_rules.push_back(pair.second.get());
        }
    }

    return filtered_rules;
}

void RuleRegistry::clear() {
    rules_.clear();
}

} // namespace rules
} // namespace cclint
