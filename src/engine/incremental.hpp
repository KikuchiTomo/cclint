#pragma once

#include <chrono>
#include <string>
#include <unordered_map>
#include <vector>

namespace cclint {
namespace engine {

/// インクリメンタル解析のためのファイル変更検出
class IncrementalAnalyzer {
public:
    /// コンストラクタ
    /// @param state_file 状態ファイルのパス
    explicit IncrementalAnalyzer(const std::string& state_file = ".cclint_state");

    /// ファイルが変更されたかチェック
    /// @param file_path ファイルパス
    /// @return 変更されていればtrue
    bool is_file_modified(const std::string& file_path) const;

    /// ファイルリストから変更されたファイルのみを抽出
    /// @param files 全ファイルリスト
    /// @return 変更されたファイルのリスト
    std::vector<std::string> filter_modified_files(
        const std::vector<std::string>& files) const;

    /// gitから変更されたファイルを取得
    /// @param base_ref 比較元のgit ref（デフォルト: HEAD）
    /// @return 変更されたファイルのリスト
    std::vector<std::string> get_git_modified_files(
        const std::string& base_ref = "HEAD") const;

    /// ファイルの状態を記録
    /// @param file_path ファイルパス
    void record_file_state(const std::string& file_path);

    /// すべての状態を保存
    void save_state();

    /// 状態をクリア
    void clear_state();

private:
    std::string state_file_;

    /// ファイルの状態（パス → 最終更新時刻）
    std::unordered_map<std::string, std::chrono::system_clock::time_point>
        file_states_;

    /// 状態ファイルを読み込む
    void load_state();

    /// ファイルの最終更新時刻を取得
    std::chrono::system_clock::time_point get_file_modification_time(
        const std::string& file_path) const;
};

} // namespace engine
} // namespace cclint
