#pragma once

#include "config/config_types.hpp"
#include "diagnostic/diagnostic.hpp"
#include "rules/rule_executor.hpp"

#include <memory>
#include <string>
#include <vector>

namespace cclint {
namespace engine {

/// ファイル解析結果
struct FileAnalysisResult {
    std::string file_path;
    bool success = false;
    std::string error_message;
    std::vector<diagnostic::Diagnostic> diagnostics;
    std::vector<rules::RuleExecutionStats> rule_stats;
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
    std::vector<FileAnalysisResult> analyze_files(
        const std::vector<std::string>& file_paths);

    /// すべての診断を取得
    /// @return すべての診断のリスト
    std::vector<diagnostic::Diagnostic> get_all_diagnostics() const;

    /// エラー数を取得
    size_t get_error_count() const;

    /// 警告数を取得
    size_t get_warning_count() const;

    /// 設定を取得
    const config::Config& get_config() const { return config_; }

private:
    config::Config config_;
    std::unique_ptr<rules::RuleExecutor> rule_executor_;
    std::vector<FileAnalysisResult> results_;

    /// ファイルがinclude/excludeパターンにマッチするかをチェック
    bool should_analyze_file(const std::string& file_path) const;

    /// ファイル内容を読み込む
    std::string read_file(const std::string& file_path) const;

    /// ルールを初期化
    void initialize_rules();
};

} // namespace engine
} // namespace cclint
