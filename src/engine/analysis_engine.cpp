#include "engine/analysis_engine.hpp"

#include <fstream>
#include <future>
#include <sstream>

#include "parser/builtin_parser.hpp"
#include "rules/rule_registry.hpp"
#include "semantic/semantic_analyzer.hpp"
#include "utils/file_utils.hpp"
#include "utils/logger.hpp"
#ifdef HAVE_LUAJIT
#include "lua/lua_rule.hpp"
#endif

namespace cclint {
namespace engine {

AnalysisEngine::AnalysisEngine(const config::Config& config)
    : config_(config),
      rule_executor_(std::make_unique<rules::RuleExecutor>()),
      cache_(config.enable_cache ? std::make_unique<cache::FileCache>(config.cache_directory)
                                 : nullptr),
      thread_pool_(config.num_threads > 1
                       ? std::make_unique<parallel::ThreadPool>(config.num_threads)
                       : nullptr),
      incremental_(config.enable_incremental ? std::make_unique<IncrementalAnalyzer>() : nullptr) {
    initialize_rules();
}

void AnalysisEngine::initialize_rules() {
    auto& registry = rules::RuleRegistry::instance();

    // Luaスクリプトルールを登録
#ifdef HAVE_LUAJIT
    for (const auto& lua_config : config_.lua_scripts) {
        try {
            // ファイル名からルール名を生成
            std::string rule_name = lua_config.path;
            size_t last_slash = rule_name.find_last_of("/\\");
            if (last_slash != std::string::npos) {
                rule_name = rule_name.substr(last_slash + 1);
            }
            size_t dot = rule_name.find_last_of('.');
            if (dot != std::string::npos) {
                rule_name = rule_name.substr(0, dot);
            }

            auto lua_rule = std::make_unique<lua::LuaRule>(lua_config.path, rule_name);

            // パラメータをvariantからstringに変換
            rules::RuleParameters params;
            for (const auto& [key, value] : lua_config.parameters) {
                if (std::holds_alternative<int>(value)) {
                    params[key] = std::to_string(std::get<int>(value));
                } else if (std::holds_alternative<std::string>(value)) {
                    params[key] = std::get<std::string>(value);
                } else if (std::holds_alternative<bool>(value)) {
                    params[key] = std::get<bool>(value) ? "true" : "false";
                }
            }

            lua_rule->initialize(params);
            registry.register_rule(std::move(lua_rule));

            utils::Logger::instance().info("Loaded Lua rule: " + lua_config.path + " (" +
                                           rule_name + ")");
        } catch (const std::exception& e) {
            utils::Logger::instance().error("Failed to load Lua rule " + lua_config.path + ": " +
                                            e.what());
        }
    }
#else
    if (!config_.lua_scripts.empty()) {
        utils::Logger::instance().warning("LuaJIT not available. " +
                                          std::to_string(config_.lua_scripts.size()) +
                                          " Lua script(s) will be ignored.");
    }
#endif

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

            utils::Logger::instance().debug("Configured rule: " + rule_config.name + " (enabled: " +
                                            (rule_config.enabled ? "true" : "false") + ")");
        } else {
            utils::Logger::instance().warning("Rule not found in registry: " + rule_config.name);
        }
    }
}

FileAnalysisResult AnalysisEngine::analyze_file(const std::string& file_path) {
    FileAnalysisResult result;
    result.file_path = file_path;

    auto start_time = std::chrono::steady_clock::now();

    // ファイルフィルタリング
    if (!should_analyze_file(file_path)) {
        utils::Logger::instance().debug("Skipping file: " + file_path);
        result.success = true;
        std::lock_guard<std::mutex> lock(results_mutex_);
        stats_.skipped_files++;
        return result;
    }

    {
        std::lock_guard<std::mutex> lock(results_mutex_);
        stats_.total_files++;
    }

    utils::Logger::instance().info("Analyzing file: " + file_path);

    try {
        // キャッシュチェック
        if (cache_) {
            std::string file_hash = cache_->calculate_file_hash(file_path);
            auto cached_entry = cache_->get(file_path, file_hash);

            if (cached_entry) {
                utils::Logger::instance().debug("Using cached result for: " + file_path);
                result.success = true;
                result.diagnostics = cached_entry->diagnostics;

                std::lock_guard<std::mutex> lock(results_mutex_);
                stats_.cached_files++;
                stats_.analyzed_files++;

                auto end_time = std::chrono::steady_clock::now();
                result.analysis_time =
                    std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
                stats_.total_time += result.analysis_time;

                results_.push_back(result);
                return result;
            }
        }

        // ファイル読み込み
        std::string content = read_file(file_path);

        // 診断エンジン
        diagnostic::DiagnosticEngine diag_engine;

        // ルール実行（テキストベース）
        auto stats = rule_executor_->execute_text_rules(file_path, content, diag_engine);

        // AST解析（C++ファイルのみ）
        if (file_path.find(".cpp") != std::string::npos ||
            file_path.find(".cc") != std::string::npos ||
            file_path.find(".cxx") != std::string::npos ||
            file_path.find(".hpp") != std::string::npos ||
            file_path.find(".h") != std::string::npos) {
            try {
                utils::Logger::instance().debug("Starting AST parsing for " + file_path);
                // Disable preprocessor temporarily to avoid issues
                parser::BuiltinParser parser(content, file_path, false);
                auto ast = parser.parse();

                if (parser.has_errors()) {
                    utils::Logger::instance().debug("AST parse warnings for " + file_path);
                }

                // セマンティック解析を実行
                if (config_.enable_semantic_analysis) {
                    utils::Logger::instance().debug("Performing semantic analysis for " +
                                                    file_path);
                    semantic::SemanticAnalyzer analyzer;
                    analyzer.analyze(ast);

                    if (analyzer.has_errors()) {
                        for (const auto& error : analyzer.errors()) {
                            utils::Logger::instance().debug("Semantic error: " + error);
                        }
                    }
                }

                utils::Logger::instance().debug("AST parsing complete, executing AST rules");
                // ASTベースのルール実行
                auto ast_stats = rule_executor_->execute_ast_rules(file_path, ast, diag_engine);
                utils::Logger::instance().debug("AST rules executed, got " +
                                                std::to_string(ast_stats.size()) + " stats");

                // 統計をマージ
                stats.insert(stats.end(), ast_stats.begin(), ast_stats.end());

            } catch (const std::exception& e) {
                utils::Logger::instance().warning("AST parsing failed for " + file_path + ": " +
                                                  e.what());
            }
        }

        // 結果を保存
        result.success = true;
        result.diagnostics = diag_engine.get_diagnostics();
        result.rule_stats = stats;

        // キャッシュに保存
        if (cache_) {
            std::string file_hash = cache_->calculate_file_hash(file_path);
            cache_->put(file_path, file_hash, result.diagnostics);
        }

        {
            std::lock_guard<std::mutex> lock(results_mutex_);
            stats_.analyzed_files++;
        }

        // 統計情報をログ出力
        if (!stats.empty()) {
            utils::Logger::instance().debug("Executed " + std::to_string(stats.size()) +
                                            " rules on " + file_path);
            for (const auto& stat : stats) {
                if (stat.failed) {
                    utils::Logger::instance().warning("Rule " + stat.rule_name +
                                                      " failed: " + stat.error_message);
                } else if (stat.diagnostics_count > 0) {
                    utils::Logger::instance().debug(
                        "Rule " + stat.rule_name + " found " +
                        std::to_string(stat.diagnostics_count) + " issues (took " +
                        std::to_string(stat.execution_time.count()) + "ms)");
                }
            }
        }

    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = e.what();
        {
            std::lock_guard<std::mutex> lock(results_mutex_);
            stats_.failed_files++;
        }
        utils::Logger::instance().error("Failed to analyze file: " + file_path + " - " + e.what());
    }

    auto end_time = std::chrono::steady_clock::now();
    result.analysis_time =
        std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    {
        std::lock_guard<std::mutex> lock(results_mutex_);
        stats_.total_time += result.analysis_time;
        results_.push_back(result);
    }

    return result;
}

std::vector<FileAnalysisResult>
AnalysisEngine::analyze_files(const std::vector<std::string>& file_paths) {
    std::vector<FileAnalysisResult> results;
    results.reserve(file_paths.size());

    // インクリメンタル解析が有効な場合、変更されたファイルのみをフィルタ
    std::vector<std::string> files_to_analyze;
    if (incremental_) {
        if (config_.use_git_diff) {
            // git diff を使って変更されたファイルを検出
            files_to_analyze = incremental_->get_git_modified_files(config_.git_base_ref);
        } else {
            // タイムスタンプベースの検出
            files_to_analyze = incremental_->filter_modified_files(file_paths);
        }

        utils::Logger::instance().info(
            "Incremental analysis: " + std::to_string(files_to_analyze.size()) + " / " +
            std::to_string(file_paths.size()) + " files to analyze");
    } else {
        files_to_analyze = file_paths;
    }

    // 並列処理が有効な場合
    if (thread_pool_ && files_to_analyze.size() > 1) {
        utils::Logger::instance().info("Analyzing " + std::to_string(files_to_analyze.size()) +
                                       " files in parallel with " +
                                       std::to_string(thread_pool_->size()) + " threads");

        std::vector<std::future<FileAnalysisResult>> futures;
        futures.reserve(files_to_analyze.size());

        // すべてのタスクをキューに追加
        for (const auto& file_path : files_to_analyze) {
            futures.push_back(thread_pool_->enqueue(
                [this, file_path]() { return this->analyze_file(file_path); }));
        }

        // 結果を収集
        for (auto& future : futures) {
            try {
                // 早期終了チェック
                if (should_stop_early()) {
                    utils::Logger::instance().warning("Stopping analysis early: max_errors (" +
                                                      std::to_string(config_.max_errors) +
                                                      ") reached");
                    stats_.stopped_early = true;
                    break;
                }

                auto result = future.get();
                results.push_back(result);

            } catch (const std::exception& e) {
                utils::Logger::instance().error("Failed to get analysis result: " +
                                                std::string(e.what()));
            }
        }

    } else {
        // シーケンシャル処理
        for (const auto& file_path : files_to_analyze) {
            // 早期終了チェック
            if (should_stop_early()) {
                utils::Logger::instance().warning("Stopping analysis early: max_errors (" +
                                                  std::to_string(config_.max_errors) + ") reached");
                stats_.stopped_early = true;
                break;
            }

            auto result = analyze_file(file_path);
            results.push_back(result);

            // インクリメンタル解析が有効な場合、状態を記録
            if (incremental_ && result.success) {
                incremental_->record_file_state(file_path);
            }
        }
    }

    // インクリメンタル解析の状態を保存
    if (incremental_) {
        incremental_->save_state();
    }

    // メモリ使用量を概算
    estimate_memory_usage();

    return results;
}

bool AnalysisEngine::should_stop_early() const {
    if (config_.max_errors <= 0) {
        return false;  // 無制限
    }

    size_t error_count = get_error_count();
    return error_count >= static_cast<size_t>(config_.max_errors);
}

std::vector<diagnostic::Diagnostic> AnalysisEngine::get_all_diagnostics() const {
    std::vector<diagnostic::Diagnostic> all_diagnostics;

    for (const auto& result : results_) {
        all_diagnostics.insert(all_diagnostics.end(), result.diagnostics.begin(),
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

void AnalysisEngine::estimate_memory_usage() {
    // 簡易的なメモリ使用量の概算
    // 診断メッセージとファイルパスの文字列長から推定
    size_t total_bytes = 0;

    for (const auto& result : results_) {
        total_bytes += result.file_path.size();
        total_bytes += result.error_message.size();

        for (const auto& diag : result.diagnostics) {
            total_bytes += diag.message.size();
            total_bytes += diag.rule_name.size();
            total_bytes += diag.location.filename.size();
            total_bytes += sizeof(diag);  // 構造体のサイズ
        }

        total_bytes += sizeof(result);
    }

    stats_.memory_usage_bytes = total_bytes;

    utils::Logger::instance().debug(
        "Estimated memory usage: " + std::to_string(total_bytes / 1024) + " KB");
}

}  // namespace engine
}  // namespace cclint
