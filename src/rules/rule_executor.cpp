#include "rules/rule_executor.hpp"

#include <exception>
#include <functional>

#include "parser/ast.hpp"
#include "rules/rule_registry.hpp"
#include "utils/logger.hpp"

namespace cclint {
namespace rules {

RuleExecutor::RuleExecutor(int timeout_ms) : timeout_ms_(timeout_ms) {}

std::vector<RuleExecutionStats>
RuleExecutor::execute_text_rules(const std::string& file_path, const std::string& content,
                                 diagnostic::DiagnosticEngine& diag_engine) {
    std::vector<RuleExecutionStats> stats;

    auto& registry = RuleRegistry::instance();
    auto enabled_rules = registry.get_enabled_rules();

    for (auto* rule : enabled_rules) {
        auto rule_stats = execute_text_rule(rule->name(), file_path, content, diag_engine);
        stats.push_back(rule_stats);
    }

    return stats;
}

std::vector<RuleExecutionStats>
RuleExecutor::execute_ast_rules(const std::string& file_path,
                                std::shared_ptr<parser::TranslationUnitNode> ast,
                                diagnostic::DiagnosticEngine& diag_engine) {
    std::vector<RuleExecutionStats> stats;

    auto& registry = RuleRegistry::instance();
    auto enabled_rules = registry.get_enabled_rules();

    utils::Logger::instance().debug("RuleExecutor::execute_ast_rules - " +
                                    std::to_string(enabled_rules.size()) + " enabled rules");

    for (auto* rule : enabled_rules) {
        utils::Logger::instance().debug("Checking rule: " + rule->name() +
                                        ", enabled=" + (rule->is_enabled() ? "true" : "false"));

        if (!rule->is_enabled()) {
            utils::Logger::instance().debug("Skipping disabled rule: " + rule->name());
            continue;
        }

        utils::Logger::instance().debug("Executing AST rule: " + rule->name());

        RuleExecutionStats rule_stats;
        rule_stats.rule_name = rule->name();

        size_t initial_diagnostic_count = diag_engine.get_diagnostics().size();
        auto start_time = std::chrono::steady_clock::now();

        try {
            // 独自ASTを使ったルール実行（ビルトインルールとLuaルール両方）
            rule->check_ast(file_path, ast, diag_engine);

            auto end_time = std::chrono::steady_clock::now();
            rule_stats.execution_time =
                std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

            rule_stats.diagnostics_count =
                diag_engine.get_diagnostics().size() - initial_diagnostic_count;

        } catch (const std::exception& e) {
            auto end_time = std::chrono::steady_clock::now();
            rule_stats.execution_time =
                std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

            rule_stats.failed = true;
            rule_stats.error_message = e.what();

            utils::Logger::instance().error("AST rule execution failed: " + rule->name() + " - " +
                                            e.what());
        }

        stats.push_back(rule_stats);
    }

    return stats;
}

std::vector<RuleExecutionStats>
RuleExecutor::execute_clang_ast_rules(clang::ASTUnit* ast_unit,
                                      diagnostic::DiagnosticEngine& diag_engine) {
    std::vector<RuleExecutionStats> stats;

    auto& registry = RuleRegistry::instance();
    auto enabled_rules = registry.get_enabled_rules();

    for (auto* rule : enabled_rules) {
        auto rule_stats = execute_ast_rule(rule->name(), ast_unit, diag_engine);
        stats.push_back(rule_stats);
    }

    return stats;
}

RuleExecutionStats RuleExecutor::execute_text_rule(const std::string& rule_name,
                                                   const std::string& file_path,
                                                   const std::string& content,
                                                   diagnostic::DiagnosticEngine& diag_engine) {
    auto& registry = RuleRegistry::instance();
    RuleBase* rule = registry.get_rule(rule_name);

    if (!rule) {
        utils::Logger::instance().warning("Rule not found: " + rule_name);
        RuleExecutionStats stats;
        stats.rule_name = rule_name;
        stats.failed = true;
        stats.error_message = "Rule not found";
        return stats;
    }

    if (!rule->is_enabled()) {
        RuleExecutionStats stats;
        stats.rule_name = rule_name;
        return stats;
    }

    return execute_with_stats(rule, diag_engine,
                              [&]() { rule->check_file(file_path, content, diag_engine); });
}

RuleExecutionStats RuleExecutor::execute_ast_rule(const std::string& rule_name,
                                                  clang::ASTUnit* ast_unit,
                                                  diagnostic::DiagnosticEngine& diag_engine) {
    auto& registry = RuleRegistry::instance();
    RuleBase* rule = registry.get_rule(rule_name);

    if (!rule) {
        utils::Logger::instance().warning("Rule not found: " + rule_name);
        RuleExecutionStats stats;
        stats.rule_name = rule_name;
        stats.failed = true;
        stats.error_message = "Rule not found";
        return stats;
    }

    if (!rule->is_enabled()) {
        RuleExecutionStats stats;
        stats.rule_name = rule_name;
        return stats;
    }

    return execute_with_stats(rule, diag_engine, [&]() { rule->check_ast(ast_unit, diag_engine); });
}

template <typename Func>
RuleExecutionStats RuleExecutor::execute_with_stats(RuleBase* rule,
                                                    diagnostic::DiagnosticEngine& diag_engine,
                                                    Func&& func) {
    RuleExecutionStats stats;
    stats.rule_name = rule->name();

    size_t initial_diagnostic_count = diag_engine.get_diagnostics().size();

    auto start_time = std::chrono::steady_clock::now();

    try {
        func();

        auto end_time = std::chrono::steady_clock::now();
        stats.execution_time =
            std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

        // タイムアウトチェック（実行後）
        if (timeout_ms_ > 0 && stats.execution_time.count() > timeout_ms_) {
            stats.timed_out = true;
            utils::Logger::instance().warning("Rule execution exceeded timeout: " + rule->name() +
                                              " (" + std::to_string(stats.execution_time.count()) +
                                              "ms > " + std::to_string(timeout_ms_) + "ms)");
        }

        stats.diagnostics_count = diag_engine.get_diagnostics().size() - initial_diagnostic_count;

    } catch (const std::exception& e) {
        auto end_time = std::chrono::steady_clock::now();
        stats.execution_time =
            std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

        stats.failed = true;
        stats.error_message = e.what();

        utils::Logger::instance().error("Rule execution failed: " + rule->name() + " - " +
                                        e.what());
    }

    return stats;
}

// 明示的なテンプレートのインスタンス化
template RuleExecutionStats
RuleExecutor::execute_with_stats<std::function<void()>>(RuleBase*, diagnostic::DiagnosticEngine&,
                                                        std::function<void()>&&);

}  // namespace rules
}  // namespace cclint
