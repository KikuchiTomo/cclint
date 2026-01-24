#pragma once

#include "diagnostic/diagnostic.hpp"

#include <chrono>
#include <optional>
#include <string>
#include <vector>

namespace cclint {
namespace cache {

/// キャッシュエントリ
struct CacheEntry {
    std::string file_path;
    std::string file_hash;  // SHA256ハッシュ
    std::chrono::system_clock::time_point timestamp;
    std::vector<diagnostic::Diagnostic> diagnostics;
};

/// ファイルキャッシュマネージャー
class FileCache {
public:
    /// コンストラクタ
    /// @param cache_dir キャッシュディレクトリ（デフォルト: .cclint_cache）
    explicit FileCache(const std::string& cache_dir = ".cclint_cache");

    /// ファイルのハッシュを計算
    /// @param file_path ファイルパス
    /// @return SHA256ハッシュ（hex文字列）
    std::string calculate_file_hash(const std::string& file_path) const;

    /// キャッシュから診断を取得
    /// @param file_path ファイルパス
    /// @param current_hash 現在のファイルハッシュ
    /// @return キャッシュエントリ（存在しない or 無効な場合はnullopt）
    std::optional<CacheEntry> get(const std::string& file_path,
                                   const std::string& current_hash) const;

    /// キャッシュに診断を保存
    /// @param file_path ファイルパス
    /// @param file_hash ファイルハッシュ
    /// @param diagnostics 診断結果
    void put(const std::string& file_path, const std::string& file_hash,
             const std::vector<diagnostic::Diagnostic>& diagnostics);

    /// キャッシュをクリア
    void clear();

    /// 古いキャッシュを削除
    /// @param max_age 最大保存期間（日数）
    void cleanup(int max_age_days = 7);

    /// キャッシュディレクトリを取得
    const std::string& get_cache_dir() const { return cache_dir_; }

private:
    std::string cache_dir_;

    /// キャッシュファイルパスを生成
    std::string get_cache_file_path(const std::string& file_path) const;

    /// ディレクトリが存在しない場合は作成
    void ensure_cache_dir_exists() const;
};

} // namespace cache
} // namespace cclint
