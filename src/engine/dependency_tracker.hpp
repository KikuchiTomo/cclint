#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>

namespace cclint {
namespace engine {

/// ファイル間の依存関係を追跡するクラス
class DependencyTracker {
public:
    DependencyTracker() = default;

    /// ファイルの依存関係を解析
    /// @param file_path ソースファイルのパス
    /// @return 依存しているファイルのリスト
    std::vector<std::string> analyze_dependencies(const std::string& file_path);

    /// ファイルが変更されたときに影響を受けるファイルを取得
    /// @param file_path 変更されたファイルのパス
    /// @return 影響を受けるファイルのリスト（再解析が必要なファイル）
    std::vector<std::string> get_affected_files(const std::string& file_path);

    /// すべての依存関係をクリア
    void clear();

    /// 依存関係グラフをダンプ（デバッグ用）
    void dump_dependencies() const;

private:
    // ファイル -> 依存しているファイルのリスト
    std::map<std::string, std::set<std::string>> dependencies_;

    // ファイル -> このファイルに依存しているファイルのリスト（逆引き）
    std::map<std::string, std::set<std::string>> reverse_dependencies_;

    /// #includeディレクティブを解析
    std::vector<std::string> parse_includes(const std::string& file_path);

    /// インクルードパスを解決
    std::string resolve_include_path(const std::string& include_directive,
                                     const std::string& current_file);

    /// 依存関係を更新
    void update_dependency(const std::string& file_path,
                          const std::vector<std::string>& dependencies);
};

} // namespace engine
} // namespace cclint
