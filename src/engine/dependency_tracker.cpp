#include "dependency_tracker.hpp"

#include <filesystem>
#include <queue>
#include <regex>

#include "utils/file_utils.hpp"
#include "utils/logger.hpp"

namespace cclint {
namespace engine {

std::vector<std::string> DependencyTracker::analyze_dependencies(const std::string& file_path) {
    auto includes = parse_includes(file_path);
    update_dependency(file_path, includes);
    return includes;
}

std::vector<std::string> DependencyTracker::parse_includes(const std::string& file_path) {
    std::vector<std::string> includes;

    try {
        std::string content = utils::FileUtils::read_file(file_path);

        // #includeディレクティブを検出する正規表現
        // #include "file.h" または #include <file.h> の形式
        std::regex include_regex(R"(^\s*#\s*include\s+[<"]([^>"]+)[>"])");

        std::istringstream iss(content);
        std::string line;

        while (std::getline(iss, line)) {
            std::smatch match;
            if (std::regex_search(line, match, include_regex)) {
                if (match.size() > 1) {
                    std::string include_file = match[1].str();
                    std::string resolved_path = resolve_include_path(include_file, file_path);
                    if (!resolved_path.empty()) {
                        includes.push_back(resolved_path);
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        utils::Logger::instance().warning("Failed to parse includes from: " + file_path + " - " +
                                          e.what());
    }

    return includes;
}

std::string DependencyTracker::resolve_include_path(const std::string& include_directive,
                                                    const std::string& current_file) {
    namespace fs = std::filesystem;

    // 現在のファイルのディレクトリ
    fs::path current_dir = fs::path(current_file).parent_path();

    // まず相対パスとして解決を試みる
    fs::path resolved = current_dir / include_directive;
    if (fs::exists(resolved)) {
        return fs::canonical(resolved).string();
    }

    // プロジェクトルートからの相対パスとして解決を試みる
    // （簡易実装: カレントディレクトリから検索）
    fs::path from_root = fs::current_path() / include_directive;
    if (fs::exists(from_root)) {
        return fs::canonical(from_root).string();
    }

    // 標準インクルードパスは解決しない（システムヘッダーは無視）
    utils::Logger::instance().debug("Could not resolve include: " + include_directive);
    return "";
}

void DependencyTracker::update_dependency(const std::string& file_path,
                                          const std::vector<std::string>& dependencies) {
    // 既存の依存関係をクリア
    auto old_deps = dependencies_[file_path];
    dependencies_[file_path].clear();

    // 新しい依存関係を追加
    for (const auto& dep : dependencies) {
        dependencies_[file_path].insert(dep);
        reverse_dependencies_[dep].insert(file_path);
    }

    // 逆引きから削除された依存関係を削除
    for (const auto& old_dep : old_deps) {
        if (dependencies_[file_path].find(old_dep) == dependencies_[file_path].end()) {
            reverse_dependencies_[old_dep].erase(file_path);
        }
    }
}

std::vector<std::string> DependencyTracker::get_affected_files(const std::string& file_path) {
    std::vector<std::string> affected;
    std::set<std::string> visited;
    std::queue<std::string> to_visit;

    to_visit.push(file_path);
    visited.insert(file_path);

    // BFSで影響を受けるファイルを収集
    while (!to_visit.empty()) {
        std::string current = to_visit.front();
        to_visit.pop();

        // このファイルに依存しているファイルを取得
        auto it = reverse_dependencies_.find(current);
        if (it != reverse_dependencies_.end()) {
            for (const auto& dependent : it->second) {
                if (visited.find(dependent) == visited.end()) {
                    visited.insert(dependent);
                    to_visit.push(dependent);
                    affected.push_back(dependent);
                }
            }
        }
    }

    return affected;
}

void DependencyTracker::clear() {
    dependencies_.clear();
    reverse_dependencies_.clear();
}

void DependencyTracker::dump_dependencies() const {
    auto& logger = utils::Logger::instance();
    logger.debug("=== Dependency Graph ===");

    for (const auto& [file, deps] : dependencies_) {
        logger.debug("File: " + file);
        for (const auto& dep : deps) {
            logger.debug("  -> " + dep);
        }
    }

    logger.debug("=== Reverse Dependencies ===");
    for (const auto& [file, deps] : reverse_dependencies_) {
        logger.debug("File: " + file);
        for (const auto& dep : deps) {
            logger.debug("  <- " + dep);
        }
    }
}

}  // namespace engine
}  // namespace cclint
