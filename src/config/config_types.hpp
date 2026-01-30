#pragma once

#include <map>
#include <string>
#include <variant>
#include <vector>

namespace config {

enum class Severity { Error, Warning, Info };

struct LuaScriptConfig {
    std::string path;
    int priority = 100;
    std::map<std::string, std::variant<int, std::string, bool>> parameters;
};

struct RuleConfig {
    std::string name;
    bool enabled = true;
    int priority = 50;
    Severity severity = Severity::Warning;
    std::map<std::string, std::variant<int, std::string, bool>> parameters;
};

struct Config {
    std::string version = "1.0";
    std::string cpp_standard = "auto";

    std::vector<std::string> include_patterns;
    std::vector<std::string> exclude_patterns;

    std::vector<RuleConfig> rules;
    std::vector<LuaScriptConfig> lua_scripts;

    std::string output_format = "text";
    int max_errors = 0;  // 0 = unlimited
    bool show_compiler_output = true;

    // パフォーマンス設定
    int num_threads = 0;  // 0 = auto-detect
    bool enable_cache = true;
    std::string cache_directory = ".cclint_cache";

    // インクリメンタル解析設定
    bool enable_incremental = false;    // インクリメンタル解析を有効化
    bool use_git_diff = false;          // git diffを使って変更ファイルを検出
    std::string git_base_ref = "HEAD";  // git diffの比較元

    // ルール実行設定
    bool parallel_rules = true;
    bool fail_fast = false;

    // 解析設定
    bool enable_semantic_analysis = true;  // セマンティック解析を有効化
};

}  // namespace config
