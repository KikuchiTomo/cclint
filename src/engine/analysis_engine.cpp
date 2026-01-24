#include "engine/analysis_engine.hpp"
#include "rules/rule_registry.hpp"
#include "rules/builtin/naming_convention.hpp"
#include "rules/builtin/header_guard.hpp"
#include "rules/builtin/max_line_length.hpp"
#include "utils/file_utils.hpp"
#include "utils/logger.hpp"

#include <fstream>
#include <sstream>

namespace cclint {
namespace engine {

AnalysisEngine::AnalysisEngine(const config::Config& config)
    : config_(config), rule_executor_(std::make_unique<rules::RuleExecutor>()) {
    initialize_rules();
}

void AnalysisEngine::initialize_rules() {
    auto& registry = rules::RuleRegistry::instance();

    // ビルトインルールを登録
    registry.register_rule(
        std::make_unique<rules::builtin::NamingConventionRule>());
    registry.register_rule(
        std::make_unique<rules::builtin::HeaderGuardRule>());
    registry.register_rule(
        std::make_unique<rules::builtin::MaxLineLengthRule>());

    // 設定からルールを有効化/無効化
    for (const auto& rule_config : config_.rules) {
        auto* rule = registry.get_rule(rule_config.name);
        if (rule) {
            rule->set_enabled(rule_config.enabled);
            rule->set_severity(rule_config.severity);

            // パラメータをvariantからstringに変換
            rules::RuleParameters params;
            for (const auto& [key, value] : rule_config.parameters) {
                if (std::holds_alternative<int>(value)) {
                    params[key] = std::to_string(std::get<int>(value));
                } else if (std::holds_alternative<std::string>(value)) {
                    params[key] = std::get<std::string>(value);
                } else if (std::holds_alternative<bool>(value)) {
                    params[key] = std::get<bool>(value) ? "true" : "false";
                }
            }

            rule->initialize(params);

            utils::Logger::instance().debug(
                "Configured rule: " + rule_config.name +
                " (enabled: " + (rule_config.enabled ? "true" : "false") + ")");
        } else {
            utils::Logger::instance().warning(
                "Rule not found in registry: " + rule_config.name);
        }
    }
}

FileAnalysisResult AnalysisEngine::analyze_file(
    const std::string& file_path) {

    FileAnalysisResult result;
    result.file_path = file_path;

    auto start_time = std::chrono::steady_clock::now();

    // ファイルフィルタリング
    if (!should_analyze_file(file_path)) {
        utils::Logger::instance().debug("Skipping file: " + file_path);
        result.success = true;
        stats_.skipped_files++;
        return result;
    }

    stats_.total_files++;

    utils::Logger::instance().info("Analyzing file: " + file_path);

    try {
        // ファイル読み込み
        std::string content = read_file(file_path);

        // 診断エンジン
        diagnostic::DiagnosticEngine diag_engine;

        // ルール実行（テキストベース）
        auto stats = rule_executor_->execute_text_rules(file_path, content,
                                                         diag_engine);

        // TODO: AST解析とASTベースのルール実行（Milestone 2+）

        // 結果を保存
        result.success = true;
        result.diagnostics = diag_engine.get_diagnostics();
        result.rule_stats = stats;

        stats_.analyzed_files++;

        // 統計情報をログ出力
        if (!stats.empty()) {
            utils::Logger::instance().debug(
                "Executed " + std::to_string(stats.size()) + " rules on " +
                file_path);
            for (const auto& stat : stats) {
                if (stat.failed) {
                    utils::Logger::instance().warning(
                        "Rule " + stat.rule_name + " failed: " +
                        stat.error_message);
                } else if (stat.diagnostics_count > 0) {
                    utils::Logger::instance().debug(
                        "Rule " + stat.rule_name + " found " +
                        std::to_string(stat.diagnostics_count) +
                        " issues (took " +
                        std::to_string(stat.execution_time.count()) + "ms)");
                }
            }
        }

    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = e.what();
        stats_.failed_files++;
        utils::Logger::instance().error("Failed to analyze file: " +
                                         file_path + " - " + e.what());
    }

    auto end_time = std::chrono::steady_clock::now();
    result.analysis_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - start_time);
    stats_.total_time += result.analysis_time;

    results_.push_back(result);
    return result;
}

std::vector<FileAnalysisResult> AnalysisEngine::analyze_files(
    const std::vector<std::string>& file_paths) {

    std::vector<FileAnalysisResult> results;
    results.reserve(file_paths.size());

    for (const auto& file_path : file_paths) {
        // 早期終了チェック
        if (should_stop_early()) {
            utils::Logger::instance().warning(
                "Stopping analysis early: max_errors (" +
                std::to_string(config_.max_errors) + ") reached");
            stats_.stopped_early = true;
            break;
        }

        auto result = analyze_file(file_path);
        results.push_back(result);
    }

    return results;
}

bool AnalysisEngine::should_stop_early() const {
    if (config_.max_errors <= 0) {
        return false;  // 無制限
    }

    size_t error_count = get_error_count();
    return error_count >= static_cast<size_t>(config_.max_errors);
}

std::vector<diagnostic::Diagnostic> AnalysisEngine::get_all_diagnostics()
    const {
    std::vector<diagnostic::Diagnostic> all_diagnostics;

    for (const auto& result : results_) {
        all_diagnostics.insert(all_diagnostics.end(),
                               result.diagnostics.begin(),
                               result.diagnostics.end());
    }

    return all_diagnostics;
}

size_t AnalysisEngine::get_error_count() const {
    size_t count = 0;
    for (const auto& result : results_) {
        for (const auto& diag : result.diagnostics) {
            if (diag.severity == diagnostic::Severity::Error) {
                count++;
            }
        }
    }
    return count;
}

size_t AnalysisEngine::get_warning_count() const {
    size_t count = 0;
    for (const auto& result : results_) {
        for (const auto& diag : result.diagnostics) {
            if (diag.severity == diagnostic::Severity::Warning) {
                count++;
            }
        }
    }
    return count;
}

bool AnalysisEngine::should_analyze_file(const std::string& file_path) const {
    // include_patterns をチェック
    if (!config_.include_patterns.empty()) {
        bool matches_include = false;
        for (const auto& pattern : config_.include_patterns) {
            if (utils::FileUtils::glob_match(file_path, pattern)) {
                matches_include = true;
                break;
            }
        }
        if (!matches_include) {
            return false;
        }
    }

    // exclude_patterns をチェック
    for (const auto& pattern : config_.exclude_patterns) {
        if (utils::FileUtils::glob_match(file_path, pattern)) {
            return false;
        }
    }

    return true;
}

std::string AnalysisEngine::read_file(const std::string& file_path) const {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + file_path);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

} // namespace engine
} // namespace cclint
