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
    // ログ出力
    static int lua_print(lua_State* L);

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
    static int lua_get_fields(lua_State* L);
    static int lua_get_field_info(lua_State* L);

    // 汎用ノードアクセス API
    static int lua_get_node_type(lua_State* L);
    static int lua_get_node_name(lua_State* L);
    static int lua_get_node_location(lua_State* L);
    static int lua_get_children(lua_State* L);
    static int lua_get_parent(lua_State* L);
    static int lua_get_source_range(lua_State* L);

    // 新しいLua Migration API
    static int lua_get_files(lua_State* L);
    static int lua_get_functions(lua_State* L);
    static int lua_get_enums(lua_State* L);
    static int lua_get_namespaces(lua_State* L);
    static int lua_get_switches(lua_State* L);
    static int lua_get_usings(lua_State* L);
    static int lua_get_includes(lua_State* L);
    static int lua_get_file_info(lua_State* L);

    // Additional C++ entity APIs
    static int lua_get_typedefs(lua_State* L);
    static int lua_get_typedef_info(lua_State* L);
    static int lua_get_variables(lua_State* L);
    static int lua_get_variable_info(lua_State* L);
    static int lua_get_macros(lua_State* L);
    static int lua_get_macro_info(lua_State* L);
    static int lua_get_if_statements(lua_State* L);
    static int lua_get_loops(lua_State* L);
    static int lua_get_try_statements(lua_State* L);
    static int lua_get_comments(lua_State* L);

    // Advanced C++ entity APIs
    static int lua_get_constructors(lua_State* L);
    static int lua_get_constructor_info(lua_State* L);
    static int lua_get_destructors(lua_State* L);
    static int lua_get_destructor_info(lua_State* L);
    static int lua_get_operators(lua_State* L);
    static int lua_get_operator_info(lua_State* L);
    static int lua_get_templates(lua_State* L);
    static int lua_get_template_info(lua_State* L);
    static int lua_get_lambdas(lua_State* L);
    static int lua_get_lambda_info(lua_State* L);
    static int lua_get_friends(lua_State* L);
    static int lua_get_static_asserts(lua_State* L);
    static int lua_get_call_graph(lua_State* L);
    static int lua_get_function_calls(lua_State* L);
    static int lua_get_callers(lua_State* L);
    static int lua_get_callees(lua_State* L);
    static int lua_get_return_statements(lua_State* L);
    static int lua_get_inheritance_tree(lua_State* L);
    static int lua_get_attributes(lua_State* L);

    // Enhanced AST APIs for complex linting rules
    static int lua_get_classes_with_info(lua_State* L);
    static int lua_get_methods_with_info(lua_State* L);
    static int lua_get_all_methods(lua_State* L);
    static int lua_get_function_info(lua_State* L);
    static int lua_get_function_parameters(lua_State* L);
    static int lua_get_classes_in_namespace(lua_State* L);
    static int lua_get_functions_in_namespace(lua_State* L);
    static int lua_get_include_details(lua_State* L);
    static int lua_get_namespace_info(lua_State* L);
    static int lua_get_class_methods_by_access(lua_State* L);
    static int lua_get_class_fields_by_access(lua_State* L);
    static int lua_is_call_allowed(lua_State* L);
    static int lua_get_all_using_declarations(lua_State* L);
    static int lua_get_base_classes(lua_State* L);

    // ヘルパー関数
    void report_diagnostic(const std::string& file_path, int line, int column,
                           const std::string& message, diagnostic::Severity severity);
};

}  // namespace lua
}  // namespace cclint
