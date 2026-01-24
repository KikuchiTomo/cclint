#pragma once

#include <chrono>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "cache/file_cache.hpp"
#include "config/config_types.hpp"
#include "diagnostic/diagnostic.hpp"
#include "engine/incremental.hpp"
#include "parallel/thread_pool.hpp"
#include "rules/rule_executor.hpp"

namespace cclint {
namespace engine {

/// ファイル解析結果
struct FileAnalysisResult {
    std::string file_path;
    bool success = false;
    std::string error_message;
    std::vector<diagnostic::Diagnostic> diagnostics;
    std::vector<rules::RuleExecutionStats> rule_stats;
    std::chrono::milliseconds analysis_time{0};
};

/// 解析エンジンの統計情報
struct AnalysisEngineStats {
    size_t total_files = 0;
    size_t analyzed_files = 0;
    size_t skipped_files = 0;
    size_t failed_files = 0;
    size_t cached_files = 0;  // キャッシュから取得したファイル数
    std::chrono::milliseconds total_time{0};
    size_t memory_usage_bytes = 0;  // メモリ使用量（概算）
    bool stopped_early = false;     // max_errorsにより早期終了したかどうか
};

/// 解析エンジン
/// ファイルの解析を統合的に管理する
class AnalysisEngine {
public:
    /// コンストラクタ
    /// @param config 設定
    explicit AnalysisEngine(const config::Config& config);

    /// 単一ファイルを解析
    /// @param file_path ファイルパス
    /// @return 解析結果
    FileAnalysisResult analyze_file(const std::string& file_path);

    /// 複数ファイルを解析
    /// @param file_paths ファイルパスのリスト
    /// @return 解析結果のリスト
    std::vector<FileAnalysisResult> analyze_files(const std::vector<std::string>& file_paths);

    /// すべての診断を取得
    /// @return すべての診断のリスト
    std::vector<diagnostic::Diagnostic> get_all_diagnostics() const;

    /// エラー数を取得
    size_t get_error_count() const;

    /// 警告数を取得
    size_t get_warning_count() const;

    /// 設定を取得
    const config::Config& get_config() const { return config_; }

    /// 統計情報を取得
    const AnalysisEngineStats& get_stats() const { return stats_; }

private:
    config::Config config_;
    std::unique_ptr<rules::RuleExecutor> rule_executor_;
    std::vector<FileAnalysisResult> results_;
    AnalysisEngineStats stats_;
    std::unique_ptr<cache::FileCache> cache_;
    std::unique_ptr<parallel::ThreadPool> thread_pool_;
    std::unique_ptr<IncrementalAnalyzer> incremental_;
    mutable std::mutex results_mutex_;  // results_とstats_の保護用

    /// ファイルがinclude/excludeパターンにマッチするかをチェック
    bool should_analyze_file(const std::string& file_path) const;

    /// ファイル内容を読み込む
    std::string read_file(const std::string& file_path) const;

    /// ルールを初期化
    void initialize_rules();

    /// 早期終了すべきかチェック
    bool should_stop_early() const;

    /// メモリ使用量を概算
    void estimate_memory_usage();
};

}  // namespace engine
}  // namespace cclint
