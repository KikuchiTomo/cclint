#include "plugin_loader.hpp"
#include "utils/logger.hpp"
#include "utils/file_utils.hpp"

#include <filesystem>

// プラットフォーム固有のヘッダー
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace cclint {
namespace rules {

// PluginHandle実装

PluginHandle::PluginHandle(void* handle, const std::string& path)
    : handle_(handle), path_(path) {
    // APIシンボルを解決
    api_.get_name = nullptr;
    api_.get_description = nullptr;
    api_.get_category = nullptr;
    api_.create_rule = nullptr;
    api_.destroy_rule = nullptr;

#ifdef _WIN32
    api_.get_name = (const char* (*)())GetProcAddress((HMODULE)handle, "cclint_plugin_get_name");
    api_.get_description = (const char* (*)())GetProcAddress((HMODULE)handle, "cclint_plugin_get_description");
    api_.get_category = (const char* (*)())GetProcAddress((HMODULE)handle, "cclint_plugin_get_category");
    api_.create_rule = (void* (*)())GetProcAddress((HMODULE)handle, "cclint_plugin_create_rule");
    api_.destroy_rule = (void (*)(void*))GetProcAddress((HMODULE)handle, "cclint_plugin_destroy_rule");
#else
    api_.get_name = (const char* (*)())dlsym(handle, "cclint_plugin_get_name");
    api_.get_description = (const char* (*)())dlsym(handle, "cclint_plugin_get_description");
    api_.get_category = (const char* (*)())dlsym(handle, "cclint_plugin_get_category");
    api_.create_rule = (void* (*)())dlsym(handle, "cclint_plugin_create_rule");
    api_.destroy_rule = (void (*)(void*))dlsym(handle, "cclint_plugin_destroy_rule");
#endif
}

PluginHandle::~PluginHandle() {
#ifdef _WIN32
    if (handle_) {
        FreeLibrary((HMODULE)handle_);
    }
#else
    if (handle_) {
        dlclose(handle_);
    }
#endif
}

std::unique_ptr<RuleBase> PluginHandle::create_rule() {
    if (!api_.create_rule) {
        return nullptr;
    }

    void* rule_ptr = api_.create_rule();
    if (!rule_ptr) {
        return nullptr;
    }

    // void*をRuleBase*にキャストして所有権を移譲
    return std::unique_ptr<RuleBase>(static_cast<RuleBase*>(rule_ptr));
}

std::string PluginHandle::get_name() const {
    if (api_.get_name) {
        return std::string(api_.get_name());
    }
    return "";
}

std::string PluginHandle::get_description() const {
    if (api_.get_description) {
        return std::string(api_.get_description());
    }
    return "";
}

std::string PluginHandle::get_category() const {
    if (api_.get_category) {
        return std::string(api_.get_category());
    }
    return "";
}

// PluginLoader実装

std::string PluginLoader::get_plugin_extension() const {
#ifdef _WIN32
    return ".dll";
#elif defined(__APPLE__)
    return ".dylib";
#else
    return ".so";
#endif
}

void* PluginLoader::load_library(const std::string& path) {
#ifdef _WIN32
    return (void*)LoadLibraryA(path.c_str());
#else
    return dlopen(path.c_str(), RTLD_LAZY);
#endif
}

void* PluginLoader::get_symbol(void* handle, const std::string& symbol_name) {
#ifdef _WIN32
    return (void*)GetProcAddress((HMODULE)handle, symbol_name.c_str());
#else
    return dlsym(handle, symbol_name.c_str());
#endif
}

void PluginLoader::unload_library(void* handle) {
#ifdef _WIN32
    FreeLibrary((HMODULE)handle);
#else
    dlclose(handle);
#endif
}

bool PluginLoader::load_plugin(const std::string& plugin_path) {
    namespace fs = std::filesystem;

    if (!fs::exists(plugin_path)) {
        utils::Logger::instance().error("Plugin file not found: " + plugin_path);
        return false;
    }

    void* handle = load_library(plugin_path);
    if (!handle) {
#ifdef _WIN32
        DWORD error = GetLastError();
        utils::Logger::instance().error("Failed to load plugin: " + plugin_path +
                                        " (error code: " + std::to_string(error) + ")");
#else
        const char* error = dlerror();
        utils::Logger::instance().error("Failed to load plugin: " + plugin_path +
                                        " (" + (error ? error : "unknown error") + ")");
#endif
        return false;
    }

    auto plugin_handle = std::make_unique<PluginHandle>(handle, plugin_path);
    std::string plugin_name = plugin_handle->get_name();

    if (plugin_name.empty()) {
        utils::Logger::instance().error(
            "Plugin does not export required symbol: " + plugin_path);
        return false;
    }

    plugins_[plugin_name] = std::move(plugin_handle);
    utils::Logger::instance().info("Loaded plugin: " + plugin_name +
                                   " from " + plugin_path);
    return true;
}

size_t PluginLoader::load_plugins_from_directory(const std::string& directory) {
    namespace fs = std::filesystem;

    if (!fs::exists(directory) || !fs::is_directory(directory)) {
        utils::Logger::instance().warning("Plugin directory not found: " + directory);
        return 0;
    }

    std::string extension = get_plugin_extension();
    size_t loaded_count = 0;

    for (const auto& entry : fs::directory_iterator(directory)) {
        if (entry.is_regular_file()) {
            std::string path = entry.path().string();
            if (path.find(extension) != std::string::npos) {
                if (load_plugin(path)) {
                    loaded_count++;
                }
            }
        }
    }

    return loaded_count;
}

std::unique_ptr<RuleBase> PluginLoader::create_rule_from_plugin(
    const std::string& plugin_name) {
    auto it = plugins_.find(plugin_name);
    if (it == plugins_.end()) {
        utils::Logger::instance().error("Plugin not found: " + plugin_name);
        return nullptr;
    }

    return it->second->create_rule();
}

std::vector<std::string> PluginLoader::get_loaded_plugins() const {
    std::vector<std::string> names;
    for (const auto& [name, handle] : plugins_) {
        names.push_back(name);
    }
    return names;
}

void PluginLoader::unload_all() {
    plugins_.clear();
    utils::Logger::instance().info("All plugins unloaded");
}

} // namespace rules
} // namespace cclint
