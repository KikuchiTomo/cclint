# cclint 詳細設計書

## 1. エントリーポイント (main.cpp)

### 1.1 main関数

```cpp
int main(int argc, char** argv) {
    try {
        // 1. コマンドライン引数解析
        cli::ArgumentParser arg_parser;
        auto args = arg_parser.parse(argc, argv);

        // 2. 設定ファイル読み込み
        config::ConfigLoader config_loader;
        auto config = config_loader.load(args.config_file);

        // 3. ロガー初期化
        util::Logger::initialize(args.verbosity);

        // 4. ルールレジストリ初期化
        rules::RuleRegistry& registry = rules::RuleRegistry::instance();

        // 5. YAML定義ルールをロード
        for (const auto& rule_name : config.enabled_rules) {
            registry.enable_rule(rule_name);
        }

        // 6. Luaエンジン初期化とスクリプトロード
        lua::LuaEngine lua_engine;
        for (const auto& script : config.lua_scripts) {
            lua_engine.load_script(script);
        }

        // 7. ソースファイル抽出
        compiler::CompilerWrapper compiler_wrapper;
        auto source_files = compiler_wrapper.extract_source_files(
            args.compiler_command);

        // 8. 解析エンジン実行
        engine::AnalysisEngine analysis_engine(config, registry, lua_engine);
        auto diagnostics = analysis_engine.analyze(source_files);

        // 9. コンパイラ実行（オプション）
        if (config.show_compiler_output) {
            auto result = compiler_wrapper.execute(args.compiler_command);
            std::cout << result.stdout_output;
            std::cerr << result.stderr_output;
        }

        // 10. 診断結果出力
        auto formatter = output::create_formatter(config.output_format);
        std::cout << formatter->format(diagnostics);

        // 11. 終了コード決定
        return diagnostics.get_error_count() > 0 ? 1 : 0;

    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 2;
    }
}
```

## 2. CLI モジュール

### 2.1 ArgumentParser クラス

```cpp
// cli/argument_parser.hpp
#pragma once
#include <string>
#include <vector>
#include <optional>

namespace cli {

struct ParsedArguments {
    std::optional<std::string> config_file;
    std::vector<std::string> compiler_command;
    std::string output_format = "text";
    int verbosity = 1;  // 0: quiet, 1: normal, 2: verbose
    bool show_help = false;
    bool show_version = false;
};

class ArgumentParser {
public:
    ParsedArguments parse(int argc, char** argv);

private:
    void parse_cclint_options(
        std::vector<std::string>& args,
        ParsedArguments& result);

    bool is_cclint_option(const std::string& arg);
    std::vector<std::string> extract_compiler_command(
        const std::vector<std::string>& args);
};

} // namespace cli
```

```cpp
// cli/argument_parser.cpp
#include "argument_parser.hpp"
#include <algorithm>
#include <stdexcept>

namespace cli {

ParsedArguments ArgumentParser::parse(int argc, char** argv) {
    ParsedArguments result;
    std::vector<std::string> args(argv + 1, argv + argc);

    if (args.empty()) {
        throw std::runtime_error("No arguments provided");
    }

    // cclintオプションを抽出
    parse_cclint_options(args, result);

    // 残りをコンパイラコマンドとして扱う
    result.compiler_command = extract_compiler_command(args);

    return result;
}

void ArgumentParser::parse_cclint_options(
    std::vector<std::string>& args,
    ParsedArguments& result)
{
    auto it = args.begin();
    while (it != args.end()) {
        if (*it == "--config") {
            if (++it == args.end()) {
                throw std::runtime_error("--config requires an argument");
            }
            result.config_file = *it;
            it = args.erase(--it, ++it);
        }
        else if (it->starts_with("--config=")) {
            result.config_file = it->substr(9);
            it = args.erase(it);
        }
        else if (*it == "--format") {
            if (++it == args.end()) {
                throw std::runtime_error("--format requires an argument");
            }
            result.output_format = *it;
            it = args.erase(--it, ++it);
        }
        else if (*it == "-v" || *it == "--verbose") {
            result.verbosity = 2;
            it = args.erase(it);
        }
        else if (*it == "-q" || *it == "--quiet") {
            result.verbosity = 0;
            it = args.erase(it);
        }
        else if (*it == "--help") {
            result.show_help = true;
            it = args.erase(it);
        }
        else if (*it == "--version") {
            result.show_version = true;
            it = args.erase(it);
        }
        else {
            ++it;
        }
    }
}

std::vector<std::string> ArgumentParser::extract_compiler_command(
    const std::vector<std::string>& args)
{
    return args;  // 残りの引数がすべてコンパイラコマンド
}

} // namespace cli
```

## 3. Config モジュール

### 3.1 設定データ構造

```cpp
// config/config_types.hpp
#pragma once
#include <string>
#include <vector>
#include <map>
#include <variant>

namespace config {

enum class Severity {
    Error,
    Warning,
    Info
};

struct RuleConfig {
    std::string name;
    bool enabled = true;
    Severity severity = Severity::Warning;
    std::map<std::string, std::variant<int, std::string, bool>> parameters;
};

struct Config {
    std::string version = "1.0";
    std::string cpp_standard = "auto";

    std::vector<std::string> include_patterns;
    std::vector<std::string> exclude_patterns;

    std::vector<RuleConfig> rules;
    std::vector<std::string> lua_scripts;

    std::string output_format = "text";
    int max_errors = 0;  // 0 = unlimited
    bool show_compiler_output = true;

    // パフォーマンス設定
    int num_threads = 0;  // 0 = auto-detect
    bool enable_cache = true;
    std::string cache_directory = ".cclint_cache";
};

} // namespace config
```

### 3.2 ConfigLoader クラス

```cpp
// config/config_loader.hpp
#pragma once
#include "config_types.hpp"
#include <optional>

namespace config {

class ConfigLoader {
public:
    Config load(const std::optional<std::string>& config_path = std::nullopt);

private:
    std::optional<std::string> find_config_file();
    Config load_yaml_config(const std::string& path);
    void apply_defaults(Config& config);
    void validate_config(const Config& config);
};

} // namespace config
```

```cpp
// config/config_loader.cpp
#include "config_loader.hpp"
#include "yaml_config.hpp"
#include "../util/file_utils.hpp"
#include <filesystem>
#include <stdexcept>

namespace config {

Config ConfigLoader::load(const std::optional<std::string>& config_path) {
    Config config;

    // 設定ファイルを探す
    std::optional<std::string> file_path = config_path;
    if (!file_path) {
        file_path = find_config_file();
    }

    // 設定ファイルが見つかったらロード
    if (file_path) {
        config = load_yaml_config(*file_path);
    }

    // デフォルト値を適用
    apply_defaults(config);

    // 設定を検証
    validate_config(config);

    return config;
}

std::optional<std::string> ConfigLoader::find_config_file() {
    namespace fs = std::filesystem;

    // 1. カレントディレクトリの .cclint.yaml
    if (fs::exists(".cclint.yaml")) {
        return ".cclint.yaml";
    }

    // 2. カレントディレクトリの cclint.yaml
    if (fs::exists("cclint.yaml")) {
        return "cclint.yaml";
    }

    // 3. プロジェクトルートを探す（.gitディレクトリを目印に）
    fs::path current = fs::current_path();
    while (current != current.root_path()) {
        if (fs::exists(current / ".git")) {
            auto config_path = current / "cclint.yaml";
            if (fs::exists(config_path)) {
                return config_path.string();
            }
            break;
        }
        current = current.parent_path();
    }

    // 4. ホームディレクトリの ~/.cclint/config.yaml
    const char* home = std::getenv("HOME");
    if (home) {
        fs::path home_config = fs::path(home) / ".cclint" / "config.yaml";
        if (fs::exists(home_config)) {
            return home_config.string();
        }
    }

    return std::nullopt;
}

Config ConfigLoader::load_yaml_config(const std::string& path) {
    YamlConfig yaml_config;
    return yaml_config.parse(path);
}

void ConfigLoader::apply_defaults(Config& config) {
    if (config.num_threads == 0) {
        config.num_threads = std::thread::hardware_concurrency();
    }
}

void ConfigLoader::validate_config(const Config& config) {
    if (config.output_format != "text" &&
        config.output_format != "json" &&
        config.output_format != "xml") {
        throw std::runtime_error(
            "Invalid output format: " + config.output_format);
    }
}

} // namespace config
```

### 3.3 YAML設定パーサー

```cpp
// config/yaml_config.hpp
#pragma once
#include "config_types.hpp"
#include <yaml-cpp/yaml.h>

namespace config {

class YamlConfig {
public:
    Config parse(const std::string& file_path);

private:
    void parse_rules(const YAML::Node& node, Config& config);
    void parse_rule_config(const YAML::Node& node, RuleConfig& rule);
    Severity parse_severity(const std::string& str);
};

} // namespace config
```

## 4. Compiler モジュール

### 4.1 CompilerWrapper クラス

```cpp
// compiler/wrapper.hpp
#pragma once
#include <string>
#include <vector>

namespace compiler {

struct CompilerResult {
    int exit_code;
    std::string stdout_output;
    std::string stderr_output;
};

class CompilerWrapper {
public:
    CompilerResult execute(const std::vector<std::string>& command);

    std::vector<std::string> extract_source_files(
        const std::vector<std::string>& command);

    std::vector<std::string> extract_compiler_flags(
        const std::vector<std::string>& command);

private:
    bool is_source_file(const std::string& file);
    bool is_compiler_flag(const std::string& arg);
};

} // namespace compiler
```

```cpp
// compiler/wrapper.cpp
#include "wrapper.hpp"
#include <cstdio>
#include <array>
#include <memory>
#include <sstream>
#include <algorithm>

namespace compiler {

CompilerResult CompilerWrapper::execute(
    const std::vector<std::string>& command)
{
    CompilerResult result;

    // コマンドを文字列に結合
    std::ostringstream cmd_stream;
    for (size_t i = 0; i < command.size(); ++i) {
        if (i > 0) cmd_stream << " ";
        cmd_stream << command[i];
    }
    std::string cmd = cmd_stream.str() + " 2>&1";

    // コマンド実行
    std::array<char, 128> buffer;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(
        popen(cmd.c_str(), "r"), pclose);

    if (!pipe) {
        throw std::runtime_error("Failed to execute compiler");
    }

    std::ostringstream output_stream;
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        output_stream << buffer.data();
    }

    result.stdout_output = output_stream.str();
    result.exit_code = pclose(pipe.release()) / 256;

    return result;
}

std::vector<std::string> CompilerWrapper::extract_source_files(
    const std::vector<std::string>& command)
{
    std::vector<std::string> source_files;

    for (const auto& arg : command) {
        if (is_source_file(arg)) {
            source_files.push_back(arg);
        }
    }

    return source_files;
}

std::vector<std::string> CompilerWrapper::extract_compiler_flags(
    const std::vector<std::string>& command)
{
    std::vector<std::string> flags;

    for (size_t i = 1; i < command.size(); ++i) {
        const auto& arg = command[i];
        if (is_compiler_flag(arg)) {
            flags.push_back(arg);
            // 次の引数が値の場合も追加
            if (arg == "-I" || arg == "-D" || arg == "-std") {
                if (i + 1 < command.size()) {
                    flags.push_back(command[++i]);
                }
            }
        }
    }

    return flags;
}

bool CompilerWrapper::is_source_file(const std::string& file) {
    return file.ends_with(".cpp") || file.ends_with(".cc") ||
           file.ends_with(".cxx") || file.ends_with(".c++") ||
           file.ends_with(".c") || file.ends_with(".h") ||
           file.ends_with(".hpp") || file.ends_with(".hh");
}

bool CompilerWrapper::is_compiler_flag(const std::string& arg) {
    return arg.starts_with("-") && !is_source_file(arg);
}

} // namespace compiler
```

## 5. Parser モジュール

### 5.1 Clangベースパーサー

```cpp
// parser/clang_parser.hpp
#pragma once
#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/ASTUnit.h>
#include <clang/Tooling/Tooling.h>
#include <memory>
#include <vector>
#include <string>

namespace parser {

class ClangParser {
public:
    std::unique_ptr<clang::ASTUnit> parse(
        const std::string& source_file,
        const std::vector<std::string>& compiler_flags);

private:
    std::vector<std::string> build_clang_args(
        const std::vector<std::string>& compiler_flags);
};

// AST訪問者の基底クラス
class ASTVisitorBase : public clang::RecursiveASTVisitor<ASTVisitorBase> {
public:
    explicit ASTVisitorBase(clang::ASTContext& context)
        : context_(context) {}

    bool VisitFunctionDecl(clang::FunctionDecl* decl);
    bool VisitVarDecl(clang::VarDecl* decl);
    bool VisitCXXRecordDecl(clang::CXXRecordDecl* decl);
    bool VisitNamespaceDecl(clang::NamespaceDecl* decl);

protected:
    clang::ASTContext& context_;
};

} // namespace parser
```

```cpp
// parser/clang_parser.cpp
#include "clang_parser.hpp"
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Tooling/Tooling.h>

namespace parser {

std::unique_ptr<clang::ASTUnit> ClangParser::parse(
    const std::string& source_file,
    const std::vector<std::string>& compiler_flags)
{
    auto args = build_clang_args(compiler_flags);

    std::unique_ptr<clang::ASTUnit> ast_unit =
        clang::tooling::buildASTFromCodeWithArgs(
            util::read_file(source_file),
            args,
            source_file
        );

    if (!ast_unit) {
        throw std::runtime_error("Failed to parse: " + source_file);
    }

    return ast_unit;
}

std::vector<std::string> ClangParser::build_clang_args(
    const std::vector<std::string>& compiler_flags)
{
    std::vector<std::string> args;

    // デフォルトのClang引数
    args.push_back("-fsyntax-only");
    args.push_back("-x");
    args.push_back("c++");

    // ユーザー指定のフラグを追加
    args.insert(args.end(), compiler_flags.begin(), compiler_flags.end());

    return args;
}

} // namespace parser
```

## 6. Rules モジュール

### 6.1 ルール基底クラス

```cpp
// rules/rule_base.hpp
#pragma once
#include "../diagnostic/diagnostic.hpp"
#include <clang/AST/ASTContext.h>
#include <string>
#include <vector>

namespace rules {

class RuleBase {
public:
    virtual ~RuleBase() = default;

    virtual std::string get_name() const = 0;
    virtual std::string get_description() const = 0;
    virtual diagnostic::Severity get_default_severity() const {
        return diagnostic::Severity::Warning;
    }

    virtual std::vector<diagnostic::Diagnostic> check(
        clang::ASTContext& ast_context) = 0;

protected:
    diagnostic::Diagnostic create_diagnostic(
        const clang::SourceLocation& location,
        const std::string& message,
        clang::ASTContext& context);
};

} // namespace rules
```

### 6.2 ルールレジストリ

```cpp
// rules/rule_registry.hpp
#pragma once
#include "rule_base.hpp"
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace rules {

class RuleRegistry {
public:
    static RuleRegistry& instance();

    void register_rule(std::unique_ptr<RuleBase> rule);
    void enable_rule(const std::string& name);
    void disable_rule(const std::string& name);
    void enable_all_rules();
    void disable_all_rules();

    std::vector<RuleBase*> get_enabled_rules() const;
    RuleBase* get_rule(const std::string& name) const;

    std::vector<std::string> get_all_rule_names() const;

private:
    RuleRegistry() = default;

    std::map<std::string, std::unique_ptr<RuleBase>> rules_;
    std::set<std::string> enabled_rules_;
};

} // namespace rules
```

### 6.3 組み込みルール例

```cpp
// rules/builtin/naming_convention.hpp
#pragma once
#include "../rule_base.hpp"
#include <regex>

namespace rules {
namespace builtin {

class NamingConventionRule : public RuleBase {
public:
    NamingConventionRule();

    std::string get_name() const override {
        return "naming_convention";
    }

    std::string get_description() const override {
        return "Check naming conventions for functions and variables";
    }

    std::vector<diagnostic::Diagnostic> check(
        clang::ASTContext& ast_context) override;

private:
    bool check_function_name(const std::string& name);
    bool check_variable_name(const std::string& name);
    bool check_class_name(const std::string& name);

    std::regex function_pattern_;
    std::regex variable_pattern_;
    std::regex class_pattern_;
};

} // namespace builtin
} // namespace rules
```

## 7. Lua モジュール

### 7.1 Luaエンジン

**設計原則**:
- LuaJIT 2.1を使用して高速な実行を実現
- 100以上のAPI関数を提供し、ユーザーが自由にチェックロジックを実装可能
- カテゴリ別にAPI登録関数を分割して保守性を向上
- サンドボックス化により安全性を確保

```cpp
// lua/lua_engine.hpp
#pragma once
#include "../diagnostic/diagnostic.hpp"
#include <lua.hpp>  // LuaJIT
#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

namespace lua {

// Luaに渡すノード情報
struct LuaNodeInfo {
    const clang::Decl* decl;
    const clang::Stmt* stmt;
    std::string type;
    clang::ASTContext* context;
};

class LuaEngine {
public:
    LuaEngine();
    ~LuaEngine();

    // Luaスクリプトをロード
    void load_script(const std::string& script_path);

    // すべてのC++ APIをLuaに登録
    void register_all_apis();

    // Luaルールを実行
    std::vector<diagnostic::Diagnostic> execute_checks(
        clang::ASTContext& ast_context);

    // エラーハンドリング
    void handle_lua_error(const std::string& error_message);

    // 診断を追加（Lua APIから呼ばれる）
    void add_diagnostic(diagnostic::Diagnostic diag);

private:
    lua_State* L_;
    std::vector<diagnostic::Diagnostic> diagnostics_;
    clang::ASTContext* current_ast_;

    // API登録関数群（カテゴリ別）
    void setup_sandbox();
    void register_diagnostic_apis();         // 診断報告API（4個）
    void register_ast_node_apis();          // ASTノード操作API（8個）
    void register_function_apis();          // 関数・メソッドAPI（10個）
    void register_variable_apis();          // 変数・フィールドAPI（8個）
    void register_class_apis();             // クラスAPI（8個）
    void register_control_flow_apis();      // 制御フローAPI（12個）
    void register_macro_apis();             // マクロAPI（5個）
    void register_file_apis();              // ファイル・位置情報API（10個）
    void register_style_apis();             // スタイル・フォーマットAPI（7個）
    void register_context_apis();           // 宣言コンテキストAPI（5個）
    void register_type_apis();              // 型情報API（8個）
    void register_comment_apis();           // コメントAPI（7個）
    void register_spacing_apis();           // 空行・スペーシングAPI（7個）
    void register_search_apis();            // 検索・走査API（10個）
    void register_utility_apis();           // ユーティリティAPI（5個）

    // ヘルパー関数
    LuaNodeInfo* check_node(lua_State* L, int index);
    void push_node(lua_State* L, const clang::Decl* decl);
    void push_node(lua_State* L, const clang::Stmt* stmt);
    void push_location(lua_State* L, const clang::SourceLocation& loc);

    // ノードキャッシュ（userdata管理用）
    std::unordered_map<const void*, int> node_refs_;
};

} // namespace lua
```

### 7.2 Lua API実装

**実装パターン**:
すべてのLua API関数は以下のパターンに従います：
1. 引数の検証とC++オブジェクトへの変換
2. Clang ASTを使った情報取得
3. 結果をLuaにプッシュ
4. エラーハンドリング

#### 7.2.1 診断報告API

```cpp
// lua/lua_api_diagnostic.cpp
#include "lua_engine.hpp"

namespace lua {
namespace api {

// report_error(location, message)
static int report_error(lua_State* L) {
    auto* engine = static_cast<LuaEngine*>(
        lua_touserdata(L, lua_upvalueindex(1)));

    // 引数検証
    luaL_checktype(L, 1, LUA_TTABLE);  // location table
    const char* message = luaL_checkstring(L, 2);

    // location table から情報取得
    lua_getfield(L, 1, "file");
    lua_getfield(L, 1, "line");
    lua_getfield(L, 1, "column");

    const char* file = lua_tostring(L, -3);
    int line = lua_tointeger(L, -2);
    int column = lua_tointeger(L, -1);
    lua_pop(L, 3);

    // 診断を作成
    diagnostic::Diagnostic diag;
    diag.severity = diagnostic::Severity::Error;
    diag.location = {file, line, column};
    diag.message = message;
    diag.rule_name = "lua_custom_rule";  // TODO: 現在のルール名を取得

    engine->add_diagnostic(diag);

    return 0;
}

// report_warning(location, message)
static int report_warning(lua_State* L) {
    // report_error と同様、severity を Warning に設定
    auto* engine = static_cast<LuaEngine*>(
        lua_touserdata(L, lua_upvalueindex(1)));

    luaL_checktype(L, 1, LUA_TTABLE);
    const char* message = luaL_checkstring(L, 2);

    lua_getfield(L, 1, "file");
    lua_getfield(L, 1, "line");
    lua_getfield(L, 1, "column");

    const char* file = lua_tostring(L, -3);
    int line = lua_tointeger(L, -2);
    int column = lua_tointeger(L, -1);
    lua_pop(L, 3);

    diagnostic::Diagnostic diag;
    diag.severity = diagnostic::Severity::Warning;
    diag.location = {file, line, column};
    diag.message = message;
    diag.rule_name = "lua_custom_rule";

    engine->add_diagnostic(diag);

    return 0;
}

// report_info(location, message)
static int report_info(lua_State* L) {
    // 同様の実装、severity を Info に設定
    // ... 省略 ...
    return 0;
}

// report_with_fix(severity, location, message, fix_text)
static int report_with_fix(lua_State* L) {
    auto* engine = static_cast<LuaEngine*>(
        lua_touserdata(L, lua_upvalueindex(1)));

    const char* severity_str = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    const char* message = luaL_checkstring(L, 3);
    const char* fix_text = luaL_checkstring(L, 4);

    // severity 文字列を enum に変換
    diagnostic::Severity severity;
    if (std::string(severity_str) == "error") {
        severity = diagnostic::Severity::Error;
    } else if (std::string(severity_str) == "warning") {
        severity = diagnostic::Severity::Warning;
    } else {
        severity = diagnostic::Severity::Info;
    }

    // location 取得
    lua_getfield(L, 2, "file");
    lua_getfield(L, 2, "line");
    lua_getfield(L, 2, "column");
    const char* file = lua_tostring(L, -3);
    int line = lua_tointeger(L, -2);
    int column = lua_tointeger(L, -1);
    lua_pop(L, 3);

    diagnostic::Diagnostic diag;
    diag.severity = severity;
    diag.location = {file, line, column};
    diag.message = message;
    diag.fix_suggestion = fix_text;
    diag.rule_name = "lua_custom_rule";

    engine->add_diagnostic(diag);

    return 0;
}

} // namespace api
} // namespace lua
```

#### 7.2.2 ASTノード操作API

```cpp
// lua/lua_api_ast.cpp
#include "lua_engine.hpp"
#include <clang/AST/Decl.h>

namespace lua {
namespace api {

// get_node_type(node)  -> string
static int get_node_type(lua_State* L) {
    auto* node_info = check_node(L, 1);

    std::string type_name;
    if (node_info->decl) {
        type_name = node_info->decl->getDeclKindName();
    } else if (node_info->stmt) {
        type_name = node_info->stmt->getStmtClassName();
    } else {
        return luaL_error(L, "Invalid node");
    }

    lua_pushstring(L, type_name.c_str());
    return 1;
}

// get_node_name(node)  -> string
static int get_node_name(lua_State* L) {
    auto* node_info = check_node(L, 1);

    if (auto* named_decl = llvm::dyn_cast_or_null<clang::NamedDecl>(node_info->decl)) {
        std::string name = named_decl->getNameAsString();
        lua_pushstring(L, name.c_str());
        return 1;
    }

    lua_pushnil(L);
    return 1;
}

// get_node_location(node)  -> {file, line, column}
static int get_node_location(lua_State* L) {
    auto* node_info = check_node(L, 1);
    auto* context = node_info->context;

    clang::SourceLocation loc;
    if (node_info->decl) {
        loc = node_info->decl->getLocation();
    } else if (node_info->stmt) {
        loc = node_info->stmt->getBeginLoc();
    } else {
        return luaL_error(L, "Invalid node");
    }

    // location table を作成
    lua_newtable(L);

    auto& source_manager = context->getSourceManager();
    std::string filename = source_manager.getFilename(loc).str();
    unsigned line = source_manager.getSpellingLineNumber(loc);
    unsigned column = source_manager.getSpellingColumnNumber(loc);

    lua_pushstring(L, filename.c_str());
    lua_setfield(L, -2, "file");

    lua_pushinteger(L, line);
    lua_setfield(L, -2, "line");

    lua_pushinteger(L, column);
    lua_setfield(L, -2, "column");

    return 1;
}

// get_children(node)  -> array of nodes
static int get_children(lua_State* L) {
    auto* node_info = check_node(L, 1);

    lua_newtable(L);  // 結果テーブル
    int index = 1;

    if (node_info->decl) {
        if (auto* dc = llvm::dyn_cast<clang::DeclContext>(node_info->decl)) {
            for (auto* child : dc->decls()) {
                push_node(L, child);
                lua_rawseti(L, -2, index++);
            }
        }
    } else if (node_info->stmt) {
        for (auto* child : node_info->stmt->children()) {
            push_node(L, child);
            lua_rawseti(L, -2, index++);
        }
    }

    return 1;
}

// get_parent(node)  -> node
static int get_parent(lua_State* L) {
    auto* node_info = check_node(L, 1);
    auto* context = node_info->context;

    if (node_info->decl) {
        auto parents = context->getParents(*node_info->decl);
        if (!parents.empty()) {
            auto& parent = parents[0];
            if (auto* decl = parent.get<clang::Decl>()) {
                push_node(L, decl);
                return 1;
            }
        }
    }

    lua_pushnil(L);
    return 1;
}

// 以降、get_next_sibling, get_prev_sibling, is_implicit なども同様のパターン
// ... 省略 ...

} // namespace api
} // namespace lua
```

#### 7.2.3 関数・メソッド関連API

```cpp
// lua/lua_api_function.cpp
#include "lua_engine.hpp"
#include <clang/AST/Decl.h>

namespace lua {
namespace api {

// has_body(node)  -> boolean
static int has_body(lua_State* L) {
    auto* node_info = check_node(L, 1);

    if (auto* func_decl = llvm::dyn_cast_or_null<clang::FunctionDecl>(node_info->decl)) {
        lua_pushboolean(L, func_decl->hasBody());
        return 1;
    }

    lua_pushboolean(L, false);
    return 1;
}

// get_access_specifier(node)  -> "public" | "private" | "protected"
static int get_access_specifier(lua_State* L) {
    auto* node_info = check_node(L, 1);

    if (auto* method = llvm::dyn_cast_or_null<clang::CXXMethodDecl>(node_info->decl)) {
        switch (method->getAccess()) {
        case clang::AS_public:
            lua_pushstring(L, "public");
            break;
        case clang::AS_protected:
            lua_pushstring(L, "protected");
            break;
        case clang::AS_private:
            lua_pushstring(L, "private");
            break;
        default:
            lua_pushstring(L, "none");
            break;
        }
        return 1;
    }

    lua_pushnil(L);
    return 1;
}

// is_static_method(node)  -> boolean
static int is_static_method(lua_State* L) {
    auto* node_info = check_node(L, 1);

    if (auto* method = llvm::dyn_cast_or_null<clang::CXXMethodDecl>(node_info->decl)) {
        lua_pushboolean(L, method->isStatic());
        return 1;
    }

    lua_pushboolean(L, false);
    return 1;
}

// 以降、get_parameters, get_return_type, is_virtual_method なども同様
// ... 省略 ...

} // namespace api
} // namespace lua
```

#### 7.2.4 空行・スペーシング関連API（重要）

```cpp
// lua/lua_api_spacing.cpp
#include "lua_engine.hpp"

namespace lua {
namespace api {

// count_blank_lines_between(node1, node2)  -> number
static int count_blank_lines_between(lua_State* L) {
    auto* node1_info = check_node(L, 1);
    auto* node2_info = check_node(L, 2);
    auto* context = node1_info->context;

    auto& source_manager = context->getSourceManager();

    // 2つのノードの位置を取得
    clang::SourceLocation loc1 = node1_info->decl ?
        node1_info->decl->getEndLoc() : node1_info->stmt->getEndLoc();
    clang::SourceLocation loc2 = node2_info->decl ?
        node2_info->decl->getBeginLoc() : node2_info->stmt->getBeginLoc();

    unsigned line1 = source_manager.getSpellingLineNumber(loc1);
    unsigned line2 = source_manager.getSpellingLineNumber(loc2);

    // 空行数を計算（間の行数 - 1）
    int blank_lines = (line2 - line1) - 1;
    if (blank_lines < 0) blank_lines = 0;

    lua_pushinteger(L, blank_lines);
    return 1;
}

// count_blank_lines_before(location)  -> number
static int count_blank_lines_before(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);

    lua_getfield(L, 1, "line");
    int line = lua_tointeger(L, -1);
    lua_pop(L, 1);

    // TODO: ソースファイルを読み込んで前の空行をカウント
    // この実装にはファイルキャッシュが必要

    lua_pushinteger(L, 0);  // 仮実装
    return 1;
}

// is_blank_line(location)  -> boolean
static int is_blank_line(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);

    lua_getfield(L, 1, "file");
    lua_getfield(L, 1, "line");
    const char* file = lua_tostring(L, -2);
    int line = lua_tointeger(L, -1);
    lua_pop(L, 2);

    // TODO: 指定された行が空行かチェック
    // ファイルキャッシュから取得

    lua_pushboolean(L, false);  // 仮実装
    return 1;
}

} // namespace api
} // namespace lua
```

#### 7.2.5 API登録

```cpp
// lua/lua_engine.cpp
#include "lua_engine.hpp"

namespace lua {

void LuaEngine::register_all_apis() {
    // 各カテゴリのAPIを登録
    register_diagnostic_apis();
    register_ast_node_apis();
    register_function_apis();
    register_variable_apis();
    register_class_apis();
    register_control_flow_apis();
    register_macro_apis();
    register_file_apis();
    register_style_apis();
    register_context_apis();
    register_type_apis();
    register_comment_apis();
    register_spacing_apis();
    register_search_apis();
    register_utility_apis();
}

void LuaEngine::register_diagnostic_apis() {
    // upvalueとして自分自身を渡す
    lua_pushlightuserdata(L_, this);
    lua_pushcclosure(L_, api::report_error, 1);
    lua_setglobal(L_, "report_error");

    lua_pushlightuserdata(L_, this);
    lua_pushcclosure(L_, api::report_warning, 1);
    lua_setglobal(L_, "report_warning");

    lua_pushlightuserdata(L_, this);
    lua_pushcclosure(L_, api::report_info, 1);
    lua_setglobal(L_, "report_info");

    lua_pushlightuserdata(L_, this);
    lua_pushcclosure(L_, api::report_with_fix, 1);
    lua_setglobal(L_, "report_with_fix");
}

void LuaEngine::register_ast_node_apis() {
    lua_pushcfunction(L_, api::get_node_type);
    lua_setglobal(L_, "get_node_type");

    lua_pushcfunction(L_, api::get_node_name);
    lua_setglobal(L_, "get_node_name");

    lua_pushcfunction(L_, api::get_node_location);
    lua_setglobal(L_, "get_node_location");

    lua_pushcfunction(L_, api::get_children);
    lua_setglobal(L_, "get_children");

    lua_pushcfunction(L_, api::get_parent);
    lua_setglobal(L_, "get_parent");

    // 以降、他の関数も同様に登録
    // ...
}

// 他のカテゴリも同様のパターンで実装
// ...

} // namespace lua
}

void LuaEngine::register_functions() {
    // C++オブジェクトをuservalueとして渡す
    lua_pushlightuserdata(L_, this);

    // 関数を登録
    lua_pushcclosure(L_, lua_report_error, 1);
    lua_setglobal(L_, "report_error");

    lua_pushlightuserdata(L_, this);
    lua_pushcclosure(L_, lua_report_warning, 1);
    lua_setglobal(L_, "report_warning");

    lua_pushcfunction(L_, lua_match_pattern);
    lua_setglobal(L_, "match_pattern");
}

void LuaEngine::setup_sandbox() {
    // 危険な関数を無効化
    lua_pushnil(L_);
    lua_setglobal(L_, "dofile");

    lua_pushnil(L_);
    lua_setglobal(L_, "loadfile");

    lua_pushnil(L_);
    lua_setglobal(L_, "os");

    // ファイルI/Oを制限
    lua_getglobal(L_, "io");
    if (lua_istable(L_, -1)) {
        lua_pushnil(L_);
        lua_setfield(L_, -2, "popen");
    }
    lua_pop(L_, 1);
}

} // namespace lua
```

## 8. Engine モジュール

### 8.1 解析エンジン

```cpp
// engine/analysis_engine.hpp
#pragma once
#include "../config/config_types.hpp"
#include "../rules/rule_registry.hpp"
#include "../lua/lua_engine.hpp"
#include "../diagnostic/reporter.hpp"
#include <string>
#include <vector>

namespace engine {

class AnalysisEngine {
public:
    AnalysisEngine(
        const config::Config& config,
        rules::RuleRegistry& rule_registry,
        lua::LuaEngine& lua_engine);

    diagnostic::DiagnosticReporter analyze(
        const std::vector<std::string>& source_files);

private:
    std::vector<diagnostic::Diagnostic> analyze_file(
        const std::string& source_file);

    bool should_analyze(const std::string& file_path);

    const config::Config& config_;
    rules::RuleRegistry& rule_registry_;
    lua::LuaEngine& lua_engine_;
};

} // namespace engine
```

```cpp
// engine/analysis_engine.cpp
#include "analysis_engine.hpp"
#include "../parser/clang_parser.hpp"
#include "../util/file_utils.hpp"
#include <thread>
#include <mutex>
#include <future>

namespace engine {

AnalysisEngine::AnalysisEngine(
    const config::Config& config,
    rules::RuleRegistry& rule_registry,
    lua::LuaEngine& lua_engine)
    : config_(config)
    , rule_registry_(rule_registry)
    , lua_engine_(lua_engine)
{
}

diagnostic::DiagnosticReporter AnalysisEngine::analyze(
    const std::vector<std::string>& source_files)
{
    diagnostic::DiagnosticReporter reporter;

    // ファイルフィルタリング
    std::vector<std::string> filtered_files;
    for (const auto& file : source_files) {
        if (should_analyze(file)) {
            filtered_files.push_back(file);
        }
    }

    // 並列解析
    std::vector<std::future<std::vector<diagnostic::Diagnostic>>> futures;

    for (const auto& file : filtered_files) {
        futures.push_back(std::async(
            std::launch::async,
            &AnalysisEngine::analyze_file,
            this,
            file
        ));
    }

    // 結果収集
    for (auto& future : futures) {
        auto diagnostics = future.get();
        for (auto& diag : diagnostics) {
            reporter.add_diagnostic(std::move(diag));
        }
    }

    reporter.sort_diagnostics();

    return reporter;
}

std::vector<diagnostic::Diagnostic> AnalysisEngine::analyze_file(
    const std::string& source_file)
{
    std::vector<diagnostic::Diagnostic> diagnostics;

    try {
        // パース
        parser::ClangParser parser;
        auto ast_unit = parser.parse(source_file, {});
        auto& ast_context = ast_unit->getASTContext();

        // C++ルール実行
        auto enabled_rules = rule_registry_.get_enabled_rules();
        for (auto* rule : enabled_rules) {
            auto diags = rule->check(ast_context);
            diagnostics.insert(diagnostics.end(), diags.begin(), diags.end());
        }

        // Luaルール実行
        auto lua_diags = lua_engine_.execute_checks(ast_context);
        diagnostics.insert(diagnostics.end(), lua_diags.begin(), lua_diags.end());

    } catch (const std::exception& e) {
        // パースエラーをログに記録
        diagnostic::Diagnostic diag;
        diag.severity = diagnostic::Severity::Error;
        diag.location = {source_file, 0, 0};
        diag.message = "Failed to parse file: " + std::string(e.what());
        diagnostics.push_back(diag);
    }

    return diagnostics;
}

bool AnalysisEngine::should_analyze(const std::string& file_path) {
    // include/excludeパターンチェック
    return util::matches_patterns(file_path, config_.include_patterns) &&
           !util::matches_patterns(file_path, config_.exclude_patterns);
}

} // namespace engine
```

## 9. Diagnostic モジュール

### 9.1 診断データ型

```cpp
// diagnostic/diagnostic.hpp
#pragma once
#include <string>
#include <optional>

namespace diagnostic {

enum class Severity {
    Error,
    Warning,
    Info
};

struct SourceLocation {
    std::string file;
    int line;
    int column;

    std::string to_string() const {
        return file + ":" + std::to_string(line) + ":" + std::to_string(column);
    }
};

struct Diagnostic {
    Severity severity;
    std::string rule_name;
    SourceLocation location;
    std::string message;
    std::optional<std::string> source_snippet;
    std::optional<std::string> fix_suggestion;

    bool operator<(const Diagnostic& other) const {
        if (location.file != other.location.file)
            return location.file < other.location.file;
        if (location.line != other.location.line)
            return location.line < other.location.line;
        return location.column < other.location.column;
    }
};

} // namespace diagnostic
```

### 9.2 診断レポーター

```cpp
// diagnostic/reporter.hpp
#pragma once
#include "diagnostic.hpp"
#include <vector>

namespace diagnostic {

class DiagnosticReporter {
public:
    void add_diagnostic(Diagnostic diag);
    void sort_diagnostics();

    const std::vector<Diagnostic>& get_diagnostics() const {
        return diagnostics_;
    }

    int get_error_count() const;
    int get_warning_count() const;
    int get_info_count() const;

private:
    std::vector<Diagnostic> diagnostics_;
};

} // namespace diagnostic
```

## 10. Output モジュール

### 10.1 出力フォーマッター

```cpp
// output/formatter.hpp
#pragma once
#include "../diagnostic/diagnostic.hpp"
#include <string>
#include <vector>
#include <memory>

namespace output {

class OutputFormatter {
public:
    virtual ~OutputFormatter() = default;
    virtual std::string format(
        const std::vector<diagnostic::Diagnostic>& diagnostics) = 0;
};

class TextFormatter : public OutputFormatter {
public:
    std::string format(
        const std::vector<diagnostic::Diagnostic>& diagnostics) override;

private:
    std::string severity_to_string(diagnostic::Severity severity);
    std::string colorize(const std::string& text, diagnostic::Severity severity);
};

class JsonFormatter : public OutputFormatter {
public:
    std::string format(
        const std::vector<diagnostic::Diagnostic>& diagnostics) override;

private:
    std::string escape_json(const std::string& str);
};

class XmlFormatter : public OutputFormatter {
public:
    std::string format(
        const std::vector<diagnostic::Diagnostic>& diagnostics) override;

private:
    std::string escape_xml(const std::string& str);
};

std::unique_ptr<OutputFormatter> create_formatter(const std::string& format);

} // namespace output
```

## 11. 実装の優先順位

### フェーズ1: 基本機能
1. CLI argument parser
2. Config loader (YAML)
3. Compiler wrapper
4. Basic Clang parser integration
5. Simple text output

### フェーズ2: ルールシステム
6. Rule base class and registry
7. 1-2 builtin rules
8. YAML-based rule configuration
9. Diagnostic reporter

### フェーズ3: Lua統合
10. Lua engine setup
11. Lua bridge implementation
12. Lua API for AST access
13. Lua sandbox

### フェーズ4: 最適化
14. Parallel analysis
15. Caching mechanism
16. JSON/XML output
17. Error handling improvements
