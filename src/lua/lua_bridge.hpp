#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "diagnostic/diagnostic.hpp"
#include "lua/lua_engine.hpp"

namespace cclint {

namespace parser {
class TranslationUnitNode;
}

namespace lua {

/// LuaとC++の橋渡しを行うクラス
/// Luaから呼び出せるC++関数を登録し、C++からLua関数を呼び出す
class LuaBridge {
public:
    /// コンストラクタ
    /// @param lua_engine Luaエンジン
    explicit LuaBridge(std::shared_ptr<LuaEngine> lua_engine);

    /// 診断エンジンを設定
    /// @param diag_engine 診断エンジン
    void set_diagnostic_engine(diagnostic::DiagnosticEngine* diag_engine);

    /// 現在のファイルパスを設定
    /// @param file_path ファイルパス
    void set_current_file(const std::string& file_path);

    /// 現在のASTを設定
    /// @param ast AST
    void set_current_ast(std::shared_ptr<parser::TranslationUnitNode> ast);

    /// C++ APIをLuaに登録
    void register_api();

    /// Lua関数を呼び出す
    /// @param function_name 関数名
    /// @param args 引数
    /// @return 成功した場合true
    bool call_function(const std::string& function_name, const std::vector<std::string>& args = {});

private:
    std::shared_ptr<LuaEngine> lua_engine_;
    diagnostic::DiagnosticEngine* diag_engine_ = nullptr;
    std::string current_file_;
    std::shared_ptr<parser::TranslationUnitNode> current_ast_;

    // Luaから呼び出されるC++関数（static）
    // 診断報告
    static int lua_report_error(lua_State* L);
    static int lua_report_warning(lua_State* L);
    static int lua_report_info(lua_State* L);

    // テキスト処理
    static int lua_get_file_content(lua_State* L);
    static int lua_match_pattern(lua_State* L);

    // AST アクセス API
    static int lua_get_classes(lua_State* L);
    static int lua_get_class_info(lua_State* L);
    static int lua_get_methods(lua_State* L);
    static int lua_get_method_info(lua_State* L);

    // 汎用ノードアクセス API
    static int lua_get_node_type(lua_State* L);
    static int lua_get_node_name(lua_State* L);
    static int lua_get_node_location(lua_State* L);
    static int lua_get_children(lua_State* L);
    static int lua_get_parent(lua_State* L);
    static int lua_get_source_range(lua_State* L);

    // ヘルパー関数
    void report_diagnostic(const std::string& file_path, int line, int column,
                           const std::string& message, diagnostic::Severity severity);
};

}  // namespace lua
}  // namespace cclint
