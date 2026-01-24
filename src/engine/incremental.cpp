#include "engine/incremental.hpp"
#include "utils/logger.hpp"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace cclint {
namespace engine {

namespace fs = std::filesystem;

IncrementalAnalyzer::IncrementalAnalyzer(const std::string& state_file)
    : state_file_(state_file) {
    load_state();
}

bool IncrementalAnalyzer::is_file_modified(
    const std::string& file_path) const {

    auto it = file_states_.find(file_path);
    if (it == file_states_.end()) {
        // 新規ファイル
        return true;
    }

    auto current_time = get_file_modification_time(file_path);
    if (current_time == std::chrono::system_clock::time_point()) {
        // ファイルが存在しない
        return false;
    }

    // タイムスタンプを比較
    return current_time > it->second;
}

std::vector<std::string> IncrementalAnalyzer::filter_modified_files(
    const std::vector<std::string>& files) const {

    std::vector<std::string> modified_files;
    for (const auto& file : files) {
        if (is_file_modified(file)) {
            modified_files.push_back(file);
        }
    }

    return modified_files;
}

std::vector<std::string> IncrementalAnalyzer::get_git_modified_files(
    const std::string& base_ref) const {

    std::vector<std::string> modified_files;

    // git diff --name-only を実行
    std::string cmd = "git diff --name-only " + base_ref + " 2>&1";
    FILE* pipe = popen(cmd.c_str(), "r");

    if (!pipe) {
        utils::Logger::instance().warning(
            "Failed to execute git command, falling back to full analysis");
        return modified_files;
    }

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        std::string line(buffer);
        // 改行を削除
        if (!line.empty() && line.back() == '\n') {
            line.pop_back();
        }

        // C/C++ファイルのみフィルタ
        if (line.find(".cpp") != std::string::npos ||
            line.find(".cc") != std::string::npos ||
            line.find(".cxx") != std::string::npos ||
            line.find(".hpp") != std::string::npos ||
            line.find(".h") != std::string::npos) {
            modified_files.push_back(line);
        }
    }

    pclose(pipe);

    utils::Logger::instance().info(
        "Found " + std::to_string(modified_files.size()) +
        " modified files via git diff");

    return modified_files;
}

void IncrementalAnalyzer::record_file_state(const std::string& file_path) {
    auto mod_time = get_file_modification_time(file_path);
    if (mod_time != std::chrono::system_clock::time_point()) {
        file_states_[file_path] = mod_time;
    }
}

void IncrementalAnalyzer::save_state() {
    try {
        std::ofstream ofs(state_file_);
        if (!ofs.is_open()) {
            utils::Logger::instance().warning(
                "Failed to save incremental state to: " + state_file_);
            return;
        }

        for (const auto& [path, time] : file_states_) {
            auto time_count = time.time_since_epoch().count();
            ofs << path << "\t" << time_count << "\n";
        }

        utils::Logger::instance().debug(
            "Saved incremental state for " +
            std::to_string(file_states_.size()) + " files");

    } catch (const std::exception& e) {
        utils::Logger::instance().warning(
            "Failed to save incremental state: " + std::string(e.what()));
    }
}

void IncrementalAnalyzer::clear_state() {
    file_states_.clear();

    try {
        if (fs::exists(state_file_)) {
            fs::remove(state_file_);
        }
    } catch (const std::exception& e) {
        utils::Logger::instance().warning(
            "Failed to clear incremental state: " + std::string(e.what()));
    }
}

void IncrementalAnalyzer::load_state() {
    try {
        if (!fs::exists(state_file_)) {
            utils::Logger::instance().debug(
                "No incremental state file found, starting fresh");
            return;
        }

        std::ifstream ifs(state_file_);
        if (!ifs.is_open()) {
            return;
        }

        std::string line;
        while (std::getline(ifs, line)) {
            std::istringstream iss(line);
            std::string path;
            long long time_count;

            if (iss >> path >> time_count) {
                auto time = std::chrono::system_clock::time_point(
                    std::chrono::system_clock::duration(time_count));
                file_states_[path] = time;
            }
        }

        utils::Logger::instance().debug(
            "Loaded incremental state for " +
            std::to_string(file_states_.size()) + " files");

    } catch (const std::exception& e) {
        utils::Logger::instance().warning(
            "Failed to load incremental state: " + std::string(e.what()));
    }
}

std::chrono::system_clock::time_point
IncrementalAnalyzer::get_file_modification_time(
    const std::string& file_path) const {

    try {
        if (!fs::exists(file_path)) {
            return std::chrono::system_clock::time_point();
        }

        auto ftime = fs::last_write_time(file_path);

        // Convert file_time to system_clock time_point
        auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            ftime - fs::file_time_type::clock::now() +
            std::chrono::system_clock::now());

        return sctp;

    } catch (const std::exception&) {
        return std::chrono::system_clock::time_point();
    }
}

} // namespace engine
} // namespace cclint
