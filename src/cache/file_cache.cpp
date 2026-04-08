#include "cache/file_cache.hpp"

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>

#include "utils/file_utils.hpp"
#include "utils/logger.hpp"
#include "utils/string_utils.hpp"

// SHA256計算用（簡易版 - 本来は専用ライブラリを使うべき）
#include <cstring>
#include <functional>

namespace cclint {
namespace cache {

namespace fs = std::filesystem;

FileCache::FileCache(const std::string& cache_dir) : cache_dir_(cache_dir) {
    ensure_cache_dir_exists();
}

std::string FileCache::calculate_file_hash(const std::string& file_path) const {
    // コンテンツベースのハッシュを試みる（より信頼性が高い）
    try {
        std::string content = utils::FileUtils::read_file(file_path);
        auto content_hash = std::hash<std::string>{}(content);

        auto file_size = fs::file_size(file_path);

        std::ostringstream oss;
        oss << std::hex << file_size << "_" << content_hash;
        return oss.str();

    } catch (const std::exception& e) {
        utils::Logger::instance().debug("Content hash failed for " + file_path + ": " +
                                        e.what() + ", falling back to size+time");
    }

    // フォールバック: ファイルサイズ+更新時刻のハッシュで代用
    try {
        auto file_size = fs::file_size(file_path);
        auto last_write = fs::last_write_time(file_path);

        // file_time_typeから直接time_since_epochを使用（クロックドメイン変換を回避）
        auto time_count = static_cast<long long>(last_write.time_since_epoch().count());

        std::ostringstream oss;
        oss << std::hex << file_size << "_" << time_count;
        return oss.str();

    } catch (const std::exception& e) {
        utils::Logger::instance().debug("File hash computation failed for " + file_path +
                                        ": " + e.what());
        return "";
    }
}

std::optional<CacheEntry> FileCache::get(const std::string& file_path,
                                         const std::string& current_hash) const {
    std::string cache_file = get_cache_file_path(file_path);

    if (!utils::FileUtils::file_exists(cache_file)) {
        return std::nullopt;
    }

    try {
        std::ifstream ifs(cache_file);
        if (!ifs.is_open()) {
            return std::nullopt;
        }

        CacheEntry entry;
        entry.file_path = file_path;

        // ハッシュを読み込む
        std::getline(ifs, entry.file_hash);

        // ハッシュが一致しない場合は無効
        if (entry.file_hash != current_hash) {
            return std::nullopt;
        }

        // タイムスタンプを読み込む
        std::string timestamp_str;
        std::getline(ifs, timestamp_str);
        auto timestamp_count = std::stoll(timestamp_str);
        entry.timestamp = std::chrono::system_clock::time_point(
            std::chrono::system_clock::duration(timestamp_count));

        // 診断情報を読み込む（簡易的なフォーマット）
        // TODO: より堅牢なシリアライゼーション

        return entry;

    } catch (const std::exception& e) {
        utils::Logger::instance().debug("Cache read failed for " + file_path + ": " + e.what());
        return std::nullopt;
    }
}

void FileCache::put(const std::string& file_path, const std::string& file_hash,
                    const std::vector<diagnostic::Diagnostic>& diagnostics) {
    std::string cache_file = get_cache_file_path(file_path);

    try {
        std::ofstream ofs(cache_file);
        if (!ofs.is_open()) {
            return;
        }

        // ハッシュを保存
        ofs << file_hash << "\n";

        // タイムスタンプを保存
        auto now = std::chrono::system_clock::now();
        ofs << now.time_since_epoch().count() << "\n";

        // 診断情報を保存（簡易的）
        ofs << diagnostics.size() << "\n";
        for (const auto& diag : diagnostics) {
            ofs << static_cast<int>(diag.severity) << "\n";
            ofs << diag.rule_name << "\n";
            ofs << diag.message << "\n";
            ofs << diag.location.filename << "\n";
            ofs << diag.location.line << "\n";
            ofs << diag.location.column << "\n";
        }

    } catch (const std::exception& e) {
        utils::Logger::instance().debug("Cache save failed for " + file_path + ": " + e.what());
    }
}

void FileCache::clear() {
    try {
        if (fs::exists(cache_dir_)) {
            fs::remove_all(cache_dir_);
            ensure_cache_dir_exists();
        }
    } catch (const std::exception& e) {
        utils::Logger::instance().debug("Cache clear failed: " + std::string(e.what()));
    }
}

void FileCache::cleanup(int max_age_days) {
    try {
        if (!fs::exists(cache_dir_)) {
            return;
        }

        auto max_age = std::chrono::hours(24 * max_age_days);
        auto now_file_time = fs::file_time_type::clock::now();

        for (const auto& entry : fs::directory_iterator(cache_dir_)) {
            if (!entry.is_regular_file()) {
                continue;
            }

            auto last_write = entry.last_write_time();
            if ((now_file_time - last_write) > max_age) {
                fs::remove(entry.path());
            }
        }
    } catch (const std::exception& e) {
        utils::Logger::instance().debug("Cache cleanup failed: " + std::string(e.what()));
    }
}

std::string FileCache::get_cache_file_path(const std::string& file_path) const {
    // ファイルパスをサニタイズしてキャッシュファイル名を生成
    std::string sanitized = file_path;

    // パス区切りを _ に置換
    for (char& c : sanitized) {
        if (c == '/' || c == '\\' || c == ':') {
            c = '_';
        }
    }

    return cache_dir_ + "/" + sanitized + ".cache";
}

void FileCache::ensure_cache_dir_exists() const {
    try {
        if (!fs::exists(cache_dir_)) {
            fs::create_directories(cache_dir_);
        }
    } catch (const std::exception& e) {
        utils::Logger::instance().debug("Cache directory creation failed: " + std::string(e.what()));
    }
}

}  // namespace cache
}  // namespace cclint
