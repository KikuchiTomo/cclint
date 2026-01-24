#include "yaml_config.hpp"

#include <sstream>
#include <stdexcept>

// TODO: yaml-cppの依存関係追加後に有効化
// #include <yaml-cpp/yaml.h>

namespace config {

Config YamlConfig::parse(std::istream& input) {
    std::string content((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    return parse_string(content);
}

Config YamlConfig::parse_string(const std::string& yaml_content) {
    // TODO: yaml-cpp依存関係追加後に実装
    // 現在はプレースホルダー実装
    throw std::runtime_error(
        "YAML parsing not yet implemented. "
        "yaml-cpp dependency will be added in Milestone 2.");

    // 将来の実装:
    /*
    try {
        YAML::Node root = YAML::Load(yaml_content);
        return parse_impl(&root);
    } catch (const YAML::Exception& e) {
        throw std::runtime_error(std::string("YAML parse error: ") + e.what());
    }
    */
}

Config YamlConfig::parse_impl(const void* yaml_node) {
    Config config = Config();  // デフォルト値で初期化

    // TODO: yaml-cpp依存関係追加後に実装
    /*
    const YAML::Node& root = *static_cast<const YAML::Node*>(yaml_node);

    // バージョン
    if (root["version"]) {
        config.version = root["version"].as<std::string>();
    }

    // C++標準
    if (root["cpp_standard"]) {
        config.cpp_standard = root["cpp_standard"].as<std::string>();
    }

    // Include/Excludeパターン
    parse_include_exclude_patterns(yaml_node, config);

    // ルール
    parse_rules(yaml_node, config);

    // Luaスクリプト
    parse_lua_scripts(yaml_node, config);

    // 出力設定
    if (root["output_format"]) {
        config.output_format = root["output_format"].as<std::string>();
    }
    if (root["max_errors"]) {
        config.max_errors = root["max_errors"].as<int>();
    }
    if (root["show_compiler_output"]) {
        config.show_compiler_output = root["show_compiler_output"].as<bool>();
    }

    // パフォーマンス設定
    parse_performance_settings(yaml_node, config);

    // ルール実行設定
    parse_rule_execution_settings(yaml_node, config);
    */

    return config;
}

void YamlConfig::parse_rules(const void* yaml_node, Config& config) {
    // TODO: yaml-cpp依存関係追加後に実装
    /*
    const YAML::Node& root = *static_cast<const YAML::Node*>(yaml_node);

    if (!root["rules"]) {
        return;
    }

    const YAML::Node& rules = root["rules"];
    for (const auto& rule_node : rules) {
        RuleConfig rule;

        if (rule_node["name"]) {
            rule.name = rule_node["name"].as<std::string>();
        }
        if (rule_node["enabled"]) {
            rule.enabled = rule_node["enabled"].as<bool>();
        }
        if (rule_node["priority"]) {
            rule.priority = rule_node["priority"].as<int>();
        }
        if (rule_node["severity"]) {
            rule.severity = parse_severity(rule_node["severity"].as<std::string>());
        }
        if (rule_node["parameters"]) {
            for (const auto& param : rule_node["parameters"]) {
                std::string key = param.first.as<std::string>();
                const YAML::Node& value_node = param.second;

                if (value_node.IsScalar()) {
                    if (value_node.Tag() == "!") {
                        // 明示的な型タグがある場合
                    } else {
                        // 型を推測
                        try {
                            rule.parameters[key] = value_node.as<int>();
                        } catch (...) {
                            try {
                                rule.parameters[key] = value_node.as<bool>();
                            } catch (...) {
                                rule.parameters[key] = value_node.as<std::string>();
                            }
                        }
                    }
                }
            }
        }

        config.rules.push_back(rule);
    }
    */
}

void YamlConfig::parse_lua_scripts(const void* yaml_node, Config& config) {
    // TODO: yaml-cpp依存関係追加後に実装
    /*
    const YAML::Node& root = *static_cast<const YAML::Node*>(yaml_node);

    if (!root["lua_scripts"]) {
        return;
    }

    const YAML::Node& scripts = root["lua_scripts"];
    for (const auto& script_node : scripts) {
        LuaScriptConfig script;

        if (script_node["path"]) {
            script.path = script_node["path"].as<std::string>();
        }
        if (script_node["priority"]) {
            script.priority = script_node["priority"].as<int>();
        }
        if (script_node["parameters"]) {
            for (const auto& param : script_node["parameters"]) {
                std::string key = param.first.as<std::string>();
                const YAML::Node& value_node = param.second;

                // ルールと同様にパラメータをパース
                if (value_node.IsScalar()) {
                    try {
                        script.parameters[key] = value_node.as<int>();
                    } catch (...) {
                        try {
                            script.parameters[key] = value_node.as<bool>();
                        } catch (...) {
                            script.parameters[key] = value_node.as<std::string>();
                        }
                    }
                }
            }
        }

        config.lua_scripts.push_back(script);
    }
    */
}

void YamlConfig::parse_include_exclude_patterns(const void* yaml_node, Config& config) {
    // TODO: yaml-cpp依存関係追加後に実装
    /*
    const YAML::Node& root = *static_cast<const YAML::Node*>(yaml_node);

    if (root["include_patterns"]) {
        for (const auto& pattern : root["include_patterns"]) {
            config.include_patterns.push_back(pattern.as<std::string>());
        }
    }

    if (root["exclude_patterns"]) {
        for (const auto& pattern : root["exclude_patterns"]) {
            config.exclude_patterns.push_back(pattern.as<std::string>());
        }
    }
    */
}

void YamlConfig::parse_performance_settings(const void* yaml_node, Config& config) {
    // TODO: yaml-cpp依存関係追加後に実装
    /*
    const YAML::Node& root = *static_cast<const YAML::Node*>(yaml_node);

    if (root["num_threads"]) {
        config.num_threads = root["num_threads"].as<int>();
    }
    if (root["enable_cache"]) {
        config.enable_cache = root["enable_cache"].as<bool>();
    }
    if (root["cache_directory"]) {
        config.cache_directory = root["cache_directory"].as<std::string>();
    }
    */
}

void YamlConfig::parse_rule_execution_settings(const void* yaml_node, Config& config) {
    // TODO: yaml-cpp依存関係追加後に実装
    /*
    const YAML::Node& root = *static_cast<const YAML::Node*>(yaml_node);

    if (root["parallel_rules"]) {
        config.parallel_rules = root["parallel_rules"].as<bool>();
    }
    if (root["fail_fast"]) {
        config.fail_fast = root["fail_fast"].as<bool>();
    }
    */
}

Severity YamlConfig::parse_severity(const std::string& severity_str) {
    if (severity_str == "error" || severity_str == "Error") {
        return Severity::Error;
    } else if (severity_str == "warning" || severity_str == "Warning") {
        return Severity::Warning;
    } else if (severity_str == "info" || severity_str == "Info") {
        return Severity::Info;
    } else {
        throw std::runtime_error("Invalid severity: " + severity_str);
    }
}

}  // namespace config
