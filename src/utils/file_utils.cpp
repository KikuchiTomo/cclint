#include "file_utils.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace utils {

namespace fs = std::filesystem;

std::string FileUtils::read_file(const std::string& path) {
    std::ifstream ifs(path);
    if (!ifs) {
        throw std::runtime_error("Failed to open file: " + path);
    }

    std::ostringstream oss;
    oss << ifs.rdbuf();
    return oss.str();
}

void FileUtils::write_file(const std::string& path,
                           const std::string& content) {
    std::ofstream ofs(path);
    if (!ofs) {
        throw std::runtime_error("Failed to open file for writing: " + path);
    }

    ofs << content;
}

bool FileUtils::file_exists(const std::string& path) {
    return fs::exists(path) && fs::is_regular_file(path);
}

bool FileUtils::directory_exists(const std::string& path) {
    return fs::exists(path) && fs::is_directory(path);
}

void FileUtils::create_directories(const std::string& path) {
    fs::create_directories(path);
}

std::string FileUtils::absolute_path(const std::string& path) {
    return fs::absolute(path).string();
}

std::string FileUtils::relative_path(const std::string& path,
                                    const std::string& base) {
    fs::path p(path);
    fs::path b(base);
    return fs::relative(p, b).string();
}

std::string FileUtils::normalize_path(const std::string& path) {
    return fs::path(path).lexically_normal().string();
}

std::string FileUtils::dirname(const std::string& path) {
    return fs::path(path).parent_path().string();
}

std::string FileUtils::basename(const std::string& path) {
    return fs::path(path).filename().string();
}

std::string FileUtils::extension(const std::string& path) {
    return fs::path(path).extension().string();
}

std::string FileUtils::stem(const std::string& path) {
    return fs::path(path).stem().string();
}

std::string FileUtils::join_path(const std::string& base,
                                const std::string& path) {
    fs::path result = fs::path(base) / path;
    return result.string();
}

std::vector<std::string> FileUtils::list_files(const std::string& directory,
                                              bool recursive) {
    std::vector<std::string> files;

    if (!directory_exists(directory)) {
        return files;
    }

    if (recursive) {
        for (const auto& entry : fs::recursive_directory_iterator(directory)) {
            if (entry.is_regular_file()) {
                files.push_back(entry.path().string());
            }
        }
    } else {
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (entry.is_regular_file()) {
                files.push_back(entry.path().string());
            }
        }
    }

    return files;
}

std::vector<std::string> FileUtils::glob(const std::string& pattern) {
    // 簡易的なグロブ実装
    // TODO: より高度なグロブパターンマッチングを実装
    std::vector<std::string> results;

    fs::path pattern_path(pattern);
    fs::path parent = pattern_path.parent_path();
    std::string filename_pattern = pattern_path.filename().string();

    if (parent.empty()) {
        parent = ".";
    }

    if (!directory_exists(parent.string())) {
        return results;
    }

    // ワイルドカード "*" のみサポート
    bool has_wildcard = filename_pattern.find('*') != std::string::npos;

    if (!has_wildcard) {
        // ワイルドカードがない場合は単純にファイルが存在するか確認
        if (file_exists(pattern)) {
            results.push_back(pattern);
        }
        return results;
    }

    // ワイルドカードがある場合はディレクトリ内を検索
    for (const auto& entry : fs::directory_iterator(parent)) {
        if (entry.is_regular_file()) {
            std::string filename = entry.path().filename().string();

            // 簡易的なパターンマッチング（"*.cpp" のような形式のみサポート）
            if (filename_pattern == "*") {
                results.push_back(entry.path().string());
            } else if (filename_pattern[0] == '*') {
                std::string suffix = filename_pattern.substr(1);
                if (filename.size() >= suffix.size() &&
                    filename.substr(filename.size() - suffix.size()) == suffix) {
                    results.push_back(entry.path().string());
                }
            } else if (filename_pattern.back() == '*') {
                std::string prefix = filename_pattern.substr(0, filename_pattern.size() - 1);
                if (filename.size() >= prefix.size() &&
                    filename.substr(0, prefix.size()) == prefix) {
                    results.push_back(entry.path().string());
                }
            }
        }
    }

    return results;
}

bool FileUtils::glob_match(const std::string& path,
                           const std::string& pattern) {
    // 簡易的なグロブマッチング実装
    // サポート: *, **

    // **/* のようなパターンは常にマッチ
    if (pattern == "**/*" || pattern == "*") {
        return true;
    }

    // パターンにワイルドカードがない場合は完全一致
    if (pattern.find('*') == std::string::npos) {
        return path == pattern;
    }

    // **/pattern のようなパターン
    if (pattern.rfind("**/", 0) == 0) {
        std::string suffix = pattern.substr(3);
        return path.find(suffix) != std::string::npos;
    }

    // *.ext のようなパターン
    if (pattern[0] == '*' && pattern.find('*', 1) == std::string::npos) {
        std::string suffix = pattern.substr(1);
        if (path.length() >= suffix.length()) {
            return path.compare(path.length() - suffix.length(),
                                suffix.length(), suffix) == 0;
        }
        return false;
    }

    // prefix* のようなパターン
    if (pattern.back() == '*' &&
        pattern.find('*') == pattern.length() - 1) {
        std::string prefix = pattern.substr(0, pattern.length() - 1);
        return path.rfind(prefix, 0) == 0;
    }

    // より複雑なパターンは正規表現を使うべきだが、
    // ここでは簡易的に部分一致でチェック
    std::string pattern_copy = pattern;
    size_t star_pos;
    while ((star_pos = pattern_copy.find('*')) != std::string::npos) {
        pattern_copy.replace(star_pos, 1, "");
    }

    return path.find(pattern_copy) != std::string::npos;
}

}  // namespace utils
