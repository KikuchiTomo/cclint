#pragma once

#include <optional>
#include <string>
#include <vector>

#include "config_types.hpp"

namespace config {

class ConfigLoader {
public:
    ConfigLoader() = default;

    // 設定ファイルを検索してロード
    // 指定されたパスから開始して、上位ディレクトリを検索
    Config load(const std::optional<std::string>& config_path = std::nullopt,
                const std::string& start_directory = ".");

    // 指定されたファイルから設定をロード
    Config load_from_file(const std::string& file_path);

    // デフォルト設定を取得
    static Config get_default_config();

    // 最後に読み込んだ設定ファイルのパスを取得
    std::optional<std::string> get_loaded_config_path() const { return loaded_config_path_; }

private:
    // 設定ファイルを検索
    std::optional<std::string> search_config_file(const std::string& start_directory);

    // 設定ファイル名のリスト（優先順）
    static constexpr const char* CONFIG_FILENAMES[] = {".cclint.yaml", ".cclint.yml", "cclint.yaml",
                                                       "cclint.yml"};

    std::optional<std::string> loaded_config_path_;
};

}  // namespace config
