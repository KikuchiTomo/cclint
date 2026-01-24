#pragma once

#include <istream>
#include <string>

#include "config_types.hpp"

namespace config {

class YamlConfig {
public:
    YamlConfig() = default;

    // YAMLストリームから設定をパース
    Config parse(std::istream& input);

    // YAML文字列から設定をパース
    Config parse_string(const std::string& yaml_content);

private:
    // YAML nodeから設定をパース（yaml-cpp使用）
    Config parse_impl(const void* yaml_node);

    // 各セクションのパース
    void parse_rules(const void* yaml_node, Config& config);
    void parse_lua_scripts(const void* yaml_node, Config& config);
    void parse_include_exclude_patterns(const void* yaml_node, Config& config);
    void parse_performance_settings(const void* yaml_node, Config& config);
    void parse_rule_execution_settings(const void* yaml_node, Config& config);

    // Severity文字列からenumへ変換
    static Severity parse_severity(const std::string& severity_str);
};

}  // namespace config
