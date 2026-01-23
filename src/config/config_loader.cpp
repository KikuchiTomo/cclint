#include "config_loader.hpp"
#include "yaml_config.hpp"

#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace config {

namespace fs = std::filesystem;

Config ConfigLoader::load(const std::optional<std::string>& config_path,
                          const std::string& start_directory) {
    std::string config_file;

    if (config_path.has_value()) {
        // 明示的に指定された設定ファイルを使用
        config_file = *config_path;
        if (!fs::exists(config_file)) {
            throw std::runtime_error("Config file not found: " + config_file);
        }
    } else {
        // 設定ファイルを検索
        auto found = search_config_file(start_directory);
        if (!found.has_value()) {
            // 設定ファイルが見つからない場合はデフォルト設定を返す
            return get_default_config();
        }
        config_file = *found;
    }

    loaded_config_path_ = config_file;
    return load_from_file(config_file);
}

Config ConfigLoader::load_from_file(const std::string& file_path) {
    if (!fs::exists(file_path)) {
        throw std::runtime_error("Config file not found: " + file_path);
    }

    std::ifstream ifs(file_path);
    if (!ifs) {
        throw std::runtime_error("Failed to open config file: " + file_path);
    }

    try {
        YamlConfig yaml_parser;
        auto config = yaml_parser.parse(ifs);
        loaded_config_path_ = file_path;
        return config;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to parse config file '" + file_path +
                                 "': " + e.what());
    }
}

Config ConfigLoader::get_default_config() {
    Config config;
    // config_types.hppのデフォルト値を使用
    // 追加のデフォルト設定があればここで設定
    return config;
}

std::optional<std::string> ConfigLoader::search_config_file(
    const std::string& start_directory) {
    fs::path current_dir = fs::absolute(start_directory);

    // 現在のディレクトリから上位ディレクトリへ遡って検索
    while (true) {
        for (const auto& filename : CONFIG_FILENAMES) {
            fs::path config_path = current_dir / filename;
            if (fs::exists(config_path) && fs::is_regular_file(config_path)) {
                return config_path.string();
            }
        }

        // 親ディレクトリへ移動
        fs::path parent = current_dir.parent_path();
        if (parent == current_dir) {
            // ルートディレクトリに到達した
            break;
        }
        current_dir = parent;
    }

    return std::nullopt;
}

}  // namespace config
