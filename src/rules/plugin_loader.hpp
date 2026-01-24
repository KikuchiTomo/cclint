#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "rules/rule_base.hpp"

namespace cclint {
namespace rules {

/// プラグインAPI構造体
struct PluginAPI {
    const char* (*get_name)();
    const char* (*get_description)();
    const char* (*get_category)();
    void* (*create_rule)();
    void (*destroy_rule)(void* rule);
};

/// プラグインハンドル
class PluginHandle {
public:
    PluginHandle(void* handle, const std::string& path);
    ~PluginHandle();

    PluginHandle(const PluginHandle&) = delete;
    PluginHandle& operator=(const PluginHandle&) = delete;

    /// プラグインからルールを作成
    std::unique_ptr<RuleBase> create_rule();

    /// プラグイン情報を取得
    std::string get_name() const;
    std::string get_description() const;
    std::string get_category() const;

    /// プラグインのパスを取得
    const std::string& get_path() const { return path_; }

private:
    void* handle_;
    std::string path_;
    PluginAPI api_;
};

/// プラグインローダークラス
class PluginLoader {
public:
    PluginLoader() = default;
    ~PluginLoader() = default;

    /// プラグインをロード
    /// @param plugin_path プラグインファイルのパス (.so, .dylib, .dll)
    /// @return 成功した場合true
    bool load_plugin(const std::string& plugin_path);

    /// ディレクトリ内のすべてのプラグインをロード
    /// @param directory プラグインディレクトリ
    /// @return ロードされたプラグイン数
    size_t load_plugins_from_directory(const std::string& directory);

    /// ロードされたプラグインからルールを作成
    /// @param plugin_name プラグイン名
    /// @return ルールインスタンス（失敗時はnullptr）
    std::unique_ptr<RuleBase> create_rule_from_plugin(const std::string& plugin_name);

    /// ロードされたプラグインの一覧を取得
    std::vector<std::string> get_loaded_plugins() const;

    /// すべてのプラグインをアンロード
    void unload_all();

private:
    std::map<std::string, std::unique_ptr<PluginHandle>> plugins_;

    /// プラグインファイルの拡張子を取得
    std::string get_plugin_extension() const;

    /// ライブラリハンドルをロード（プラットフォーム固有）
    void* load_library(const std::string& path);

    /// シンボルを解決（プラットフォーム固有）
    void* get_symbol(void* handle, const std::string& symbol_name);

    /// ライブラリをアンロード（プラットフォーム固有）
    void unload_library(void* handle);
};

}  // namespace rules
}  // namespace cclint
