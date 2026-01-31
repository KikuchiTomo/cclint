#include "lua/lua_bridge.hpp"

#include <regex>
#include <sstream>

#include "parser/ast.hpp"
#include "utils/file_utils.hpp"
#include "utils/logger.hpp"

namespace cclint {
namespace lua {

#ifdef HAVE_LUAJIT

// グローバルなブリッジインスタンス（Luaのコールバック用）
static LuaBridge* g_bridge = nullptr;

LuaBridge::LuaBridge(std::shared_ptr<LuaEngine> lua_engine) : lua_engine_(lua_engine) {
    g_bridge = this;
}

void LuaBridge::set_diagnostic_engine(diagnostic::DiagnosticEngine* diag_engine) {
    diag_engine_ = diag_engine;
}

void LuaBridge::set_current_file(const std::string& file_path) {
    current_file_ = file_path;
}

void LuaBridge::set_current_ast(std::shared_ptr<parser::TranslationUnitNode> ast) {
    current_ast_ = ast;
    // Update g_bridge to this instance when AST is set
    // This ensures the correct bridge is used when Lua calls API functions
    g_bridge = this;
}

void LuaBridge::register_api() {
    if (!lua_engine_ || !LuaEngine::is_available()) {
        return;
    }

    lua_State* L = lua_engine_->get_state();

    // Luaのprint関数を上書きしてログに統合
    lua_pushcfunction(L, lua_print);
    lua_setglobal(L, "print");

    // グローバルテーブル "cclint" を作成
    lua_newtable(L);

    // 診断報告API
    lua_pushcfunction(L, lua_report_error);
    lua_setfield(L, -2, "report_error");

    lua_pushcfunction(L, lua_report_warning);
    lua_setfield(L, -2, "report_warning");

    lua_pushcfunction(L, lua_report_info);
    lua_setfield(L, -2, "report_info");

    // ユーティリティAPI
    lua_pushcfunction(L, lua_get_file_content);
    lua_setfield(L, -2, "get_file_content");

    lua_pushcfunction(L, lua_match_pattern);
    lua_setfield(L, -2, "match_pattern");

    // AST アクセス API
    lua_pushcfunction(L, lua_get_classes);
    lua_setfield(L, -2, "get_classes");

    lua_pushcfunction(L, lua_get_class_info);
    lua_setfield(L, -2, "get_class_info");

    lua_pushcfunction(L, lua_get_methods);
    lua_setfield(L, -2, "get_methods");

    lua_pushcfunction(L, lua_get_method_info);
    lua_setfield(L, -2, "get_method_info");

    lua_pushcfunction(L, lua_get_fields);
    lua_setfield(L, -2, "get_fields");

    lua_pushcfunction(L, lua_get_field_info);
    lua_setfield(L, -2, "get_field_info");

    // 汎用ノードアクセス API
    lua_pushcfunction(L, lua_get_node_type);
    lua_setfield(L, -2, "get_node_type");

    lua_pushcfunction(L, lua_get_node_name);
    lua_setfield(L, -2, "get_node_name");

    lua_pushcfunction(L, lua_get_node_location);
    lua_setfield(L, -2, "get_node_location");

    lua_pushcfunction(L, lua_get_children);
    lua_setfield(L, -2, "get_children");

    lua_pushcfunction(L, lua_get_parent);
    lua_setfield(L, -2, "get_parent");

    lua_pushcfunction(L, lua_get_source_range);
    lua_setfield(L, -2, "get_source_range");

    // 新しいLua Migration API
    lua_pushcfunction(L, lua_get_files);
    lua_setfield(L, -2, "get_files");

    lua_pushcfunction(L, lua_get_functions);
    lua_setfield(L, -2, "get_functions");

    lua_pushcfunction(L, lua_get_enums);
    lua_setfield(L, -2, "get_enums");

    lua_pushcfunction(L, lua_get_namespaces);
    lua_setfield(L, -2, "get_namespaces");

    lua_pushcfunction(L, lua_get_switches);
    lua_setfield(L, -2, "get_switches");

    lua_pushcfunction(L, lua_get_usings);
    lua_setfield(L, -2, "get_usings");

    lua_pushcfunction(L, lua_get_includes);
    lua_setfield(L, -2, "get_includes");

    lua_pushcfunction(L, lua_get_file_info);
    lua_setfield(L, -2, "get_file_info");

    // Additional C++ entity APIs
    lua_pushcfunction(L, lua_get_typedefs);
    lua_setfield(L, -2, "get_typedefs");

    lua_pushcfunction(L, lua_get_typedef_info);
    lua_setfield(L, -2, "get_typedef_info");

    lua_pushcfunction(L, lua_get_variables);
    lua_setfield(L, -2, "get_variables");

    lua_pushcfunction(L, lua_get_variable_info);
    lua_setfield(L, -2, "get_variable_info");

    lua_pushcfunction(L, lua_get_macros);
    lua_setfield(L, -2, "get_macros");

    lua_pushcfunction(L, lua_get_macro_info);
    lua_setfield(L, -2, "get_macro_info");

    lua_pushcfunction(L, lua_get_if_statements);
    lua_setfield(L, -2, "get_if_statements");

    lua_pushcfunction(L, lua_get_loops);
    lua_setfield(L, -2, "get_loops");

    lua_pushcfunction(L, lua_get_try_statements);
    lua_setfield(L, -2, "get_try_statements");

    lua_pushcfunction(L, lua_get_comments);
    lua_setfield(L, -2, "get_comments");

    // Advanced C++ entity APIs
    lua_pushcfunction(L, lua_get_constructors);
    lua_setfield(L, -2, "get_constructors");

    lua_pushcfunction(L, lua_get_constructor_info);
    lua_setfield(L, -2, "get_constructor_info");

    lua_pushcfunction(L, lua_get_destructors);
    lua_setfield(L, -2, "get_destructors");

    lua_pushcfunction(L, lua_get_destructor_info);
    lua_setfield(L, -2, "get_destructor_info");

    lua_pushcfunction(L, lua_get_operators);
    lua_setfield(L, -2, "get_operators");

    lua_pushcfunction(L, lua_get_operator_info);
    lua_setfield(L, -2, "get_operator_info");

    lua_pushcfunction(L, lua_get_templates);
    lua_setfield(L, -2, "get_templates");

    lua_pushcfunction(L, lua_get_template_info);
    lua_setfield(L, -2, "get_template_info");

    lua_pushcfunction(L, lua_get_lambdas);
    lua_setfield(L, -2, "get_lambdas");

    lua_pushcfunction(L, lua_get_lambda_info);
    lua_setfield(L, -2, "get_lambda_info");

    lua_pushcfunction(L, lua_get_friends);
    lua_setfield(L, -2, "get_friends");

    lua_pushcfunction(L, lua_get_static_asserts);
    lua_setfield(L, -2, "get_static_asserts");

    lua_pushcfunction(L, lua_get_call_graph);
    lua_setfield(L, -2, "get_call_graph");

    lua_pushcfunction(L, lua_get_function_calls);
    lua_setfield(L, -2, "get_function_calls");

    lua_pushcfunction(L, lua_get_callers);
    lua_setfield(L, -2, "get_callers");

    lua_pushcfunction(L, lua_get_callees);
    lua_setfield(L, -2, "get_callees");

    lua_pushcfunction(L, lua_get_return_statements);
    lua_setfield(L, -2, "get_return_statements");

    lua_pushcfunction(L, lua_get_inheritance_tree);
    lua_setfield(L, -2, "get_inheritance_tree");

    lua_pushcfunction(L, lua_get_attributes);
    lua_setfield(L, -2, "get_attributes");

    // Enhanced AST APIs for complex linting rules
    lua_pushcfunction(L, lua_get_classes_with_info);
    lua_setfield(L, -2, "get_classes_with_info");

    lua_pushcfunction(L, lua_get_methods_with_info);
    lua_setfield(L, -2, "get_methods_with_info");

    lua_pushcfunction(L, lua_get_all_methods);
    lua_setfield(L, -2, "get_all_methods");

    lua_pushcfunction(L, lua_get_function_info);
    lua_setfield(L, -2, "get_function_info");

    lua_pushcfunction(L, lua_get_function_parameters);
    lua_setfield(L, -2, "get_function_parameters");

    lua_pushcfunction(L, lua_get_classes_in_namespace);
    lua_setfield(L, -2, "get_classes_in_namespace");

    lua_pushcfunction(L, lua_get_functions_in_namespace);
    lua_setfield(L, -2, "get_functions_in_namespace");

    lua_pushcfunction(L, lua_get_include_details);
    lua_setfield(L, -2, "get_include_details");

    lua_pushcfunction(L, lua_get_namespace_info);
    lua_setfield(L, -2, "get_namespace_info");

    lua_pushcfunction(L, lua_get_class_methods_by_access);
    lua_setfield(L, -2, "get_class_methods_by_access");

    lua_pushcfunction(L, lua_get_class_fields_by_access);
    lua_setfield(L, -2, "get_class_fields_by_access");

    lua_pushcfunction(L, lua_is_call_allowed);
    lua_setfield(L, -2, "is_call_allowed");

    lua_pushcfunction(L, lua_get_all_using_declarations);
    lua_setfield(L, -2, "get_all_using_declarations");

    lua_pushcfunction(L, lua_get_base_classes);
    lua_setfield(L, -2, "get_base_classes");

    // グローバルに設定
    lua_setglobal(L, "cclint");

    utils::Logger::instance().debug("Lua C++ API registered");
}

bool LuaBridge::call_function(const std::string& function_name,
                              const std::vector<std::string>& args) {
    return lua_engine_->call_function(function_name, args);
}

void LuaBridge::report_diagnostic(const std::string& file_path, int line, int column,
                                  const std::string& message, diagnostic::Severity severity) {
    if (!diag_engine_) {
        utils::Logger::instance().warning("Diagnostic engine not set in LuaBridge");
        return;
    }

    diagnostic::Diagnostic diag;
    diag.severity = severity;
    diag.rule_name = "lua-rule";
    diag.message = message;
    diag.location.filename = file_path;
    diag.location.line = line;
    diag.location.column = column;

    diag_engine_->add_diagnostic(diag);
}

// Luaから呼び出されるC++関数の実装

int LuaBridge::lua_print(lua_State* L) {
    int n = lua_gettop(L);  // 引数の数
    std::ostringstream oss;

    for (int i = 1; i <= n; i++) {
        if (i > 1) {
            oss << "\t";
        }
        if (lua_isstring(L, i)) {
            oss << lua_tostring(L, i);
        } else if (lua_isnil(L, i)) {
            oss << "nil";
        } else if (lua_isboolean(L, i)) {
            oss << (lua_toboolean(L, i) ? "true" : "false");
        } else {
            oss << luaL_typename(L, i);
        }
    }

    utils::Logger::instance().info("[Lua] " + oss.str());
    return 0;
}

int LuaBridge::lua_report_error(lua_State* L) {
    if (!g_bridge || !g_bridge->diag_engine_) {
        return 0;
    }

    int line = luaL_checkinteger(L, 1);
    int column = luaL_checkinteger(L, 2);
    const char* message = luaL_checkstring(L, 3);

    g_bridge->report_diagnostic(g_bridge->current_file_, line, column, message,
                                diagnostic::Severity::Error);

    return 0;
}

int LuaBridge::lua_report_warning(lua_State* L) {
    if (!g_bridge || !g_bridge->diag_engine_) {
        return 0;
    }

    int line = luaL_checkinteger(L, 1);
    int column = luaL_checkinteger(L, 2);
    const char* message = luaL_checkstring(L, 3);

    g_bridge->report_diagnostic(g_bridge->current_file_, line, column, message,
                                diagnostic::Severity::Warning);

    return 0;
}

int LuaBridge::lua_report_info(lua_State* L) {
    if (!g_bridge || !g_bridge->diag_engine_) {
        return 0;
    }

    int line = luaL_checkinteger(L, 1);
    int column = luaL_checkinteger(L, 2);
    const char* message = luaL_checkstring(L, 3);

    g_bridge->report_diagnostic(g_bridge->current_file_, line, column, message,
                                diagnostic::Severity::Info);

    return 0;
}

int LuaBridge::lua_get_file_content(lua_State* L) {
    const char* file_path = luaL_checkstring(L, 1);

    try {
        std::string content = utils::FileUtils::read_file(file_path);
        lua_pushstring(L, content.c_str());
        return 1;
    } catch (const std::exception& e) {
        luaL_error(L, "Failed to read file: %s", e.what());
        return 0;
    }
}

int LuaBridge::lua_match_pattern(lua_State* L) {
    const char* text = luaL_checkstring(L, 1);
    const char* pattern = luaL_checkstring(L, 2);

    try {
        std::regex regex_pattern(pattern);
        std::string text_str(text);

        std::smatch match;
        if (std::regex_search(text_str, match, regex_pattern)) {
            lua_pushboolean(L, 1);

            // マッチした文字列をテーブルで返す
            lua_newtable(L);
            for (size_t i = 0; i < match.size(); ++i) {
                lua_pushinteger(L, i);
                lua_pushstring(L, match[i].str().c_str());
                lua_settable(L, -3);
            }
            return 2;  // boolean と table を返す
        } else {
            lua_pushboolean(L, 0);
            return 1;
        }
    } catch (const std::exception& e) {
        luaL_error(L, "Regex error: %s", e.what());
        return 0;
    }
}

// AST アクセス API の実装

int LuaBridge::lua_get_classes(lua_State* L) {
    if (!g_bridge || !g_bridge->current_ast_) {
        lua_newtable(L);  // 空のテーブルを返す
        return 1;
    }

    lua_newtable(L);  // 結果テーブル
    int index = 1;

    // ASTを再帰的に走査してクラスを収集
    std::function<void(std::shared_ptr<parser::ASTNode>)> collect_classes;
    collect_classes = [&](std::shared_ptr<parser::ASTNode> node) {
        if (!node)
            return;

        if (node->type == parser::ASTNodeType::Class) {
            auto class_node = std::dynamic_pointer_cast<parser::ClassNode>(node);
            if (class_node) {
                lua_pushinteger(L, index++);
                lua_pushstring(L, class_node->name.c_str());
                lua_settable(L, -3);
            }
        }

        for (const auto& child : node->children) {
            collect_classes(child);
        }
    };

    collect_classes(g_bridge->current_ast_);
    return 1;
}

int LuaBridge::lua_get_class_info(lua_State* L) {
    const char* class_name = luaL_checkstring(L, 1);

    if (!g_bridge || !g_bridge->current_ast_) {
        lua_pushnil(L);
        return 1;
    }

    // クラスを検索
    std::shared_ptr<parser::ClassNode> found_class;
    std::function<void(std::shared_ptr<parser::ASTNode>)> find_class;
    find_class = [&](std::shared_ptr<parser::ASTNode> node) {
        if (!node || found_class)
            return;

        if (node->type == parser::ASTNodeType::Class) {
            auto class_node = std::dynamic_pointer_cast<parser::ClassNode>(node);
            if (class_node && class_node->name == class_name) {
                found_class = class_node;
                return;
            }
        }

        for (const auto& child : node->children) {
            find_class(child);
        }
    };

    find_class(g_bridge->current_ast_);

    if (!found_class) {
        lua_pushnil(L);
        return 1;
    }

    // クラス情報をテーブルで返す
    lua_newtable(L);

    lua_pushstring(L, "name");
    lua_pushstring(L, found_class->name.c_str());
    lua_settable(L, -3);

    lua_pushstring(L, "is_struct");
    lua_pushboolean(L, found_class->is_struct);
    lua_settable(L, -3);

    lua_pushstring(L, "line");
    lua_pushinteger(L, found_class->position.line);
    lua_settable(L, -3);

    return 1;
}

int LuaBridge::lua_get_methods(lua_State* L) {
    const char* class_name = luaL_checkstring(L, 1);

    if (!g_bridge || !g_bridge->current_ast_) {
        lua_newtable(L);
        return 1;
    }

    // クラスを検索
    std::shared_ptr<parser::ClassNode> found_class;
    std::function<void(std::shared_ptr<parser::ASTNode>)> find_class;
    find_class = [&](std::shared_ptr<parser::ASTNode> node) {
        if (!node || found_class)
            return;

        if (node->type == parser::ASTNodeType::Class) {
            auto class_node = std::dynamic_pointer_cast<parser::ClassNode>(node);
            if (class_node && class_node->name == class_name) {
                found_class = class_node;
                return;
            }
        }

        for (const auto& child : node->children) {
            find_class(child);
        }
    };

    find_class(g_bridge->current_ast_);

    if (!found_class) {
        lua_newtable(L);
        return 1;
    }

    // メソッド一覧をテーブルで返す
    lua_newtable(L);
    int index = 1;

    for (const auto& child : found_class->children) {
        if (child->type == parser::ASTNodeType::Function ||
            child->type == parser::ASTNodeType::Method) {
            auto func_node = std::dynamic_pointer_cast<parser::FunctionNode>(child);
            if (func_node) {
                lua_pushinteger(L, index++);
                lua_pushstring(L, func_node->name.c_str());
                lua_settable(L, -3);
            }
        }
    }

    return 1;
}

int LuaBridge::lua_get_method_info(lua_State* L) {
    const char* class_name = luaL_checkstring(L, 1);
    const char* method_name = luaL_checkstring(L, 2);

    if (!g_bridge || !g_bridge->current_ast_) {
        lua_pushnil(L);
        return 1;
    }

    // クラスを検索
    std::shared_ptr<parser::ClassNode> found_class;
    std::function<void(std::shared_ptr<parser::ASTNode>)> find_class;
    find_class = [&](std::shared_ptr<parser::ASTNode> node) {
        if (!node || found_class)
            return;

        if (node->type == parser::ASTNodeType::Class) {
            auto class_node = std::dynamic_pointer_cast<parser::ClassNode>(node);
            if (class_node && class_node->name == class_name) {
                found_class = class_node;
                return;
            }
        }

        for (const auto& child : node->children) {
            find_class(child);
        }
    };

    find_class(g_bridge->current_ast_);

    if (!found_class) {
        lua_pushnil(L);
        return 1;
    }

    // メソッドを検索
    std::shared_ptr<parser::FunctionNode> found_method;
    for (const auto& child : found_class->children) {
        if (child->type == parser::ASTNodeType::Function ||
            child->type == parser::ASTNodeType::Method) {
            auto func_node = std::dynamic_pointer_cast<parser::FunctionNode>(child);
            if (func_node && func_node->name == method_name) {
                found_method = func_node;
                break;
            }
        }
    }

    if (!found_method) {
        lua_pushnil(L);
        return 1;
    }

    // メソッド情報をテーブルで返す
    lua_newtable(L);

    lua_pushstring(L, "name");
    lua_pushstring(L, found_method->name.c_str());
    lua_settable(L, -3);

    lua_pushstring(L, "return_type");
    lua_pushstring(L, found_method->return_type.c_str());
    lua_settable(L, -3);

    lua_pushstring(L, "line");
    lua_pushinteger(L, found_method->position.line);
    lua_settable(L, -3);

    lua_pushstring(L, "is_const");
    lua_pushboolean(L, found_method->is_const);
    lua_settable(L, -3);

    lua_pushstring(L, "is_static");
    lua_pushboolean(L, found_method->is_static);
    lua_settable(L, -3);

    lua_pushstring(L, "is_virtual");
    lua_pushboolean(L, found_method->is_virtual);
    lua_settable(L, -3);

    // アクセス指定子
    const char* access_str = "none";
    switch (found_method->access) {
        case parser::AccessSpecifier::Public:
            access_str = "public";
            break;
        case parser::AccessSpecifier::Protected:
            access_str = "protected";
            break;
        case parser::AccessSpecifier::Private:
            access_str = "private";
            break;
        default:
            break;
    }

    lua_pushstring(L, "access");
    lua_pushstring(L, access_str);
    lua_settable(L, -3);

    return 1;
}

int LuaBridge::lua_get_fields(lua_State* L) {
    const char* class_name = luaL_checkstring(L, 1);

    if (!g_bridge || !g_bridge->current_ast_) {
        lua_newtable(L);
        return 1;
    }

    // クラスを検索
    std::shared_ptr<parser::ClassNode> found_class;
    std::function<void(std::shared_ptr<parser::ASTNode>)> find_class;
    find_class = [&](std::shared_ptr<parser::ASTNode> node) {
        if (!node || found_class)
            return;

        if (node->type == parser::ASTNodeType::Class) {
            auto class_node = std::dynamic_pointer_cast<parser::ClassNode>(node);
            if (class_node && class_node->name == class_name) {
                found_class = class_node;
                return;
            }
        }

        for (const auto& child : node->children) {
            find_class(child);
        }
    };

    find_class(g_bridge->current_ast_);

    if (!found_class) {
        lua_newtable(L);
        return 1;
    }

    // フィールド一覧をテーブルで返す
    lua_newtable(L);
    int index = 1;

    for (const auto& child : found_class->children) {
        if (child->type == parser::ASTNodeType::Field) {
            auto field_node = std::dynamic_pointer_cast<parser::FieldNode>(child);
            if (field_node) {
                lua_pushinteger(L, index++);
                lua_pushstring(L, field_node->name.c_str());
                lua_settable(L, -3);
            }
        }
    }

    return 1;
}

int LuaBridge::lua_get_field_info(lua_State* L) {
    const char* class_name = luaL_checkstring(L, 1);
    const char* field_name = luaL_checkstring(L, 2);

    if (!g_bridge || !g_bridge->current_ast_) {
        lua_pushnil(L);
        return 1;
    }

    // クラスを検索
    std::shared_ptr<parser::ClassNode> found_class;
    std::function<void(std::shared_ptr<parser::ASTNode>)> find_class;
    find_class = [&](std::shared_ptr<parser::ASTNode> node) {
        if (!node || found_class)
            return;

        if (node->type == parser::ASTNodeType::Class) {
            auto class_node = std::dynamic_pointer_cast<parser::ClassNode>(node);
            if (class_node && class_node->name == class_name) {
                found_class = class_node;
                return;
            }
        }

        for (const auto& child : node->children) {
            find_class(child);
        }
    };

    find_class(g_bridge->current_ast_);

    if (!found_class) {
        lua_pushnil(L);
        return 1;
    }

    // フィールドを検索
    std::shared_ptr<parser::FieldNode> found_field;
    for (const auto& child : found_class->children) {
        if (child->type == parser::ASTNodeType::Field) {
            auto field_node = std::dynamic_pointer_cast<parser::FieldNode>(child);
            if (field_node && field_node->name == field_name) {
                found_field = field_node;
                break;
            }
        }
    }

    if (!found_field) {
        lua_pushnil(L);
        return 1;
    }

    // フィールド情報をテーブルで返す
    lua_newtable(L);

    lua_pushstring(L, "name");
    lua_pushstring(L, found_field->name.c_str());
    lua_settable(L, -3);

    lua_pushstring(L, "type");
    lua_pushstring(L, found_field->type_name.c_str());
    lua_settable(L, -3);

    lua_pushstring(L, "line");
    lua_pushinteger(L, found_field->position.line);
    lua_settable(L, -3);

    lua_pushstring(L, "column");
    lua_pushinteger(L, found_field->position.column);
    lua_settable(L, -3);

    lua_pushstring(L, "is_const");
    lua_pushboolean(L, found_field->is_const);
    lua_settable(L, -3);

    lua_pushstring(L, "is_static");
    lua_pushboolean(L, found_field->is_static);
    lua_settable(L, -3);

    lua_pushstring(L, "is_mutable");
    lua_pushboolean(L, found_field->is_mutable);
    lua_settable(L, -3);

    // アクセス指定子
    const char* access_str = "none";
    switch (found_field->access) {
        case parser::AccessSpecifier::Public:
            access_str = "public";
            break;
        case parser::AccessSpecifier::Protected:
            access_str = "protected";
            break;
        case parser::AccessSpecifier::Private:
            access_str = "private";
            break;
        default:
            break;
    }

    lua_pushstring(L, "access");
    lua_pushstring(L, access_str);
    lua_settable(L, -3);

    // アクセス指定子を数値でも返す（互換性のため）
    int access_value = 0;  // public
    switch (found_field->access) {
        case parser::AccessSpecifier::Public:
            access_value = 0;
            break;
        case parser::AccessSpecifier::Protected:
            access_value = 1;
            break;
        case parser::AccessSpecifier::Private:
            access_value = 2;
            break;
        default:
            break;
    }

    lua_pushstring(L, "access_value");
    lua_pushinteger(L, access_value);
    lua_settable(L, -3);

    return 1;
}

// 汎用ノードアクセス API の実装

int LuaBridge::lua_get_node_type(lua_State* L) {
    // ノードポインタをlight userdataとして受け取る
    if (!lua_islightuserdata(L, 1)) {
        luaL_error(L, "Expected node pointer as light userdata");
        return 0;
    }

    auto* node = static_cast<parser::ASTNode*>(lua_touserdata(L, 1));
    if (!node) {
        lua_pushnil(L);
        return 1;
    }

    // ノードの型を文字列で返す
    std::string type_name = node->get_type_name();
    lua_pushstring(L, type_name.c_str());
    return 1;
}

int LuaBridge::lua_get_node_name(lua_State* L) {
    if (!lua_islightuserdata(L, 1)) {
        luaL_error(L, "Expected node pointer as light userdata");
        return 0;
    }

    auto* node = static_cast<parser::ASTNode*>(lua_touserdata(L, 1));
    if (!node) {
        lua_pushnil(L);
        return 1;
    }

    lua_pushstring(L, node->name.c_str());
    return 1;
}

int LuaBridge::lua_get_node_location(lua_State* L) {
    if (!lua_islightuserdata(L, 1)) {
        luaL_error(L, "Expected node pointer as light userdata");
        return 0;
    }

    auto* node = static_cast<parser::ASTNode*>(lua_touserdata(L, 1));
    if (!node) {
        lua_pushnil(L);
        return 1;
    }

    // 位置情報をテーブルで返す
    lua_newtable(L);

    lua_pushstring(L, "file");
    lua_pushstring(L, node->position.filename.c_str());
    lua_settable(L, -3);

    lua_pushstring(L, "line");
    lua_pushinteger(L, node->position.line);
    lua_settable(L, -3);

    lua_pushstring(L, "column");
    lua_pushinteger(L, node->position.column);
    lua_settable(L, -3);

    return 1;
}

int LuaBridge::lua_get_children(lua_State* L) {
    if (!lua_islightuserdata(L, 1)) {
        luaL_error(L, "Expected node pointer as light userdata");
        return 0;
    }

    auto* node = static_cast<parser::ASTNode*>(lua_touserdata(L, 1));
    if (!node) {
        lua_newtable(L);
        return 1;
    }

    // 子ノードをテーブルで返す
    lua_newtable(L);
    int index = 1;

    for (const auto& child : node->children) {
        lua_pushinteger(L, index++);
        // 子ノードをlight userdataとして返す
        lua_pushlightuserdata(L, child.get());
        lua_settable(L, -3);
    }

    return 1;
}

int LuaBridge::lua_get_parent(lua_State* L) {
    if (!lua_islightuserdata(L, 1)) {
        luaL_error(L, "Expected node pointer as light userdata");
        return 0;
    }

    auto* node = static_cast<parser::ASTNode*>(lua_touserdata(L, 1));
    if (!node || !g_bridge || !g_bridge->current_ast_) {
        lua_pushnil(L);
        return 1;
    }

    // 親ノードを見つけるために、ASTを走査する
    // （注意: これはO(n)の操作なので、パフォーマンスが重要な場合は親マップを事前構築すべき）
    std::shared_ptr<parser::ASTNode> parent_node;
    std::function<bool(std::shared_ptr<parser::ASTNode>, std::shared_ptr<parser::ASTNode>)>
        find_parent;

    find_parent = [&](std::shared_ptr<parser::ASTNode> current,
                      std::shared_ptr<parser::ASTNode> parent) -> bool {
        if (current.get() == node) {
            parent_node = parent;
            return true;
        }
        for (const auto& child : current->children) {
            if (find_parent(child, current)) {
                return true;
            }
        }
        return false;
    };

    // ルートから検索
    for (const auto& child : g_bridge->current_ast_->children) {
        if (find_parent(child, g_bridge->current_ast_)) {
            break;
        }
    }

    if (parent_node) {
        lua_pushlightuserdata(L, parent_node.get());
    } else {
        lua_pushnil(L);
    }

    return 1;
}

int LuaBridge::lua_get_source_range(lua_State* L) {
    if (!lua_islightuserdata(L, 1)) {
        luaL_error(L, "Expected node pointer as light userdata");
        return 0;
    }

    auto* node = static_cast<parser::ASTNode*>(lua_touserdata(L, 1));
    if (!node) {
        lua_pushnil(L);
        return 1;
    }

    // ソース範囲をテーブルで返す
    // 現在のASTには終了位置がないため、開始位置のみを返す
    // 将来的には、終了位置も追加すべき
    lua_newtable(L);

    lua_pushstring(L, "start");
    lua_newtable(L);
    lua_pushstring(L, "line");
    lua_pushinteger(L, node->position.line);
    lua_settable(L, -3);
    lua_pushstring(L, "column");
    lua_pushinteger(L, node->position.column);
    lua_settable(L, -3);
    lua_settable(L, -3);

    // 終了位置は未実装（将来の拡張）
    lua_pushstring(L, "end");
    lua_newtable(L);
    lua_pushstring(L, "line");
    lua_pushinteger(L, node->position.line);  // 仮に開始位置と同じにする
    lua_settable(L, -3);
    lua_pushstring(L, "column");
    lua_pushinteger(L, node->position.column);
    lua_settable(L, -3);
    lua_settable(L, -3);

    return 1;
}

// 新しいLua Migration API実装

int LuaBridge::lua_get_files(lua_State* L) {
    if (!g_bridge || !g_bridge->current_ast_) {
        lua_newtable(L);
        return 1;
    }

    // ファイル配列を作成
    lua_newtable(L);

    std::string file_path = g_bridge->current_file_;

    lua_pushnumber(L, 1);
    lua_newtable(L);

    // path
    lua_pushstring(L, "path");
    lua_pushstring(L, file_path.c_str());
    lua_settable(L, -3);

    // name
    lua_pushstring(L, "name");
    size_t pos = file_path.find_last_of("/\\");
    std::string name = (pos == std::string::npos) ? file_path : file_path.substr(pos + 1);
    lua_pushstring(L, name.c_str());
    lua_settable(L, -3);

    // extension
    lua_pushstring(L, "extension");
    pos = file_path.find_last_of('.');
    std::string ext = (pos == std::string::npos) ? "" : file_path.substr(pos + 1);
    lua_pushstring(L, ext.c_str());
    lua_settable(L, -3);

    // is_header
    lua_pushstring(L, "is_header");
    bool is_header = (ext == "h" || ext == "hpp" || ext == "hxx" || ext == "hh");
    lua_pushboolean(L, is_header);
    lua_settable(L, -3);

    // is_implementation
    lua_pushstring(L, "is_implementation");
    bool is_impl = (ext == "cpp" || ext == "cc" || ext == "cxx" || ext == "c");
    lua_pushboolean(L, is_impl);
    lua_settable(L, -3);

    lua_settable(L, -3);

    return 1;
}

int LuaBridge::lua_get_functions(lua_State* L) {
    if (!g_bridge || !g_bridge->current_ast_) {
        lua_newtable(L);
        return 1;
    }

    // 関数を収集するヘルパー
    std::vector<std::shared_ptr<parser::FunctionNode>> functions;
    std::function<void(std::shared_ptr<parser::ASTNode>)> collect;
    collect = [&](std::shared_ptr<parser::ASTNode> node) {
        if (!node)
            return;

        if (node->type == parser::ASTNodeType::Function) {
            functions.push_back(std::static_pointer_cast<parser::FunctionNode>(node));
        }

        for (auto& child : node->children) {
            collect(child);
        }
    };

    collect(g_bridge->current_ast_);

    // Luaテーブルを作成
    lua_newtable(L);

    for (size_t i = 0; i < functions.size(); i++) {
        lua_pushnumber(L, i + 1);
        lua_newtable(L);

        auto func = functions[i];

        lua_pushstring(L, "name");
        lua_pushstring(L, func->name.c_str());
        lua_settable(L, -3);

        lua_pushstring(L, "return_type");
        lua_pushstring(L, func->return_type.c_str());
        lua_settable(L, -3);

        lua_pushstring(L, "line");
        lua_pushnumber(L, func->position.line);
        lua_settable(L, -3);

        lua_pushstring(L, "file");
        lua_pushstring(L, func->position.filename.c_str());
        lua_settable(L, -3);

        lua_pushstring(L, "is_static");
        lua_pushboolean(L, func->is_static);
        lua_settable(L, -3);

        lua_pushstring(L, "is_virtual");
        lua_pushboolean(L, func->is_virtual);
        lua_settable(L, -3);

        lua_pushstring(L, "is_const");
        lua_pushboolean(L, func->is_const);
        lua_settable(L, -3);

        lua_settable(L, -3);
    }

    return 1;
}

int LuaBridge::lua_get_enums(lua_State* L) {
    if (!g_bridge || !g_bridge->current_ast_) {
        lua_newtable(L);
        return 1;
    }

    // enumを収集
    std::vector<std::shared_ptr<parser::EnumNode>> enums;
    std::function<void(std::shared_ptr<parser::ASTNode>)> collect;
    collect = [&](std::shared_ptr<parser::ASTNode> node) {
        if (!node)
            return;

        if (node->type == parser::ASTNodeType::Enum) {
            enums.push_back(std::static_pointer_cast<parser::EnumNode>(node));
        }

        for (auto& child : node->children) {
            collect(child);
        }
    };

    collect(g_bridge->current_ast_);

    // Luaテーブルを作成
    lua_newtable(L);

    for (size_t i = 0; i < enums.size(); i++) {
        lua_pushnumber(L, i + 1);
        lua_newtable(L);

        auto enum_node = enums[i];

        lua_pushstring(L, "name");
        lua_pushstring(L, enum_node->name.c_str());
        lua_settable(L, -3);

        lua_pushstring(L, "is_class");
        lua_pushboolean(L, enum_node->is_class);
        lua_settable(L, -3);

        lua_pushstring(L, "line");
        lua_pushnumber(L, enum_node->position.line);
        lua_settable(L, -3);

        // values配列
        lua_pushstring(L, "values");
        lua_newtable(L);
        int value_index = 1;
        for (auto& child : enum_node->children) {
            if (child->type == parser::ASTNodeType::EnumConstant) {
                lua_pushnumber(L, value_index++);
                lua_pushstring(L, child->name.c_str());
                lua_settable(L, -3);
            }
        }
        lua_settable(L, -3);

        lua_settable(L, -3);
    }

    return 1;
}

int LuaBridge::lua_get_namespaces(lua_State* L) {
    if (!g_bridge || !g_bridge->current_ast_) {
        lua_newtable(L);
        return 1;
    }

    // namespaceを収集
    std::vector<std::shared_ptr<parser::NamespaceNode>> namespaces;
    std::function<void(std::shared_ptr<parser::ASTNode>)> collect;
    collect = [&](std::shared_ptr<parser::ASTNode> node) {
        if (!node)
            return;

        if (node->type == parser::ASTNodeType::Namespace) {
            namespaces.push_back(std::static_pointer_cast<parser::NamespaceNode>(node));
        }

        for (auto& child : node->children) {
            collect(child);
        }
    };

    collect(g_bridge->current_ast_);

    // Luaテーブルを作成
    lua_newtable(L);

    for (size_t i = 0; i < namespaces.size(); i++) {
        lua_pushnumber(L, i + 1);
        lua_newtable(L);

        auto ns_node = namespaces[i];

        lua_pushstring(L, "name");
        lua_pushstring(L, ns_node->name.c_str());
        lua_settable(L, -3);

        lua_pushstring(L, "line");
        lua_pushnumber(L, ns_node->position.line);
        lua_settable(L, -3);

        lua_settable(L, -3);
    }

    return 1;
}

int LuaBridge::lua_get_switches(lua_State* L) {
    if (!g_bridge || !g_bridge->current_ast_) {
        lua_newtable(L);
        return 1;
    }

    // switch文を収集
    std::vector<std::shared_ptr<parser::SwitchStatementNode>> switches;
    std::function<void(std::shared_ptr<parser::ASTNode>)> collect;
    collect = [&](std::shared_ptr<parser::ASTNode> node) {
        if (!node)
            return;

        if (node->type == parser::ASTNodeType::SwitchStatement) {
            switches.push_back(std::static_pointer_cast<parser::SwitchStatementNode>(node));
        }

        for (auto& child : node->children) {
            collect(child);
        }
    };

    collect(g_bridge->current_ast_);

    // Luaテーブルを作成
    lua_newtable(L);

    for (size_t i = 0; i < switches.size(); i++) {
        lua_pushnumber(L, i + 1);
        lua_newtable(L);

        auto switch_node = switches[i];

        lua_pushstring(L, "line");
        lua_pushnumber(L, switch_node->position.line);
        lua_settable(L, -3);

        lua_pushstring(L, "has_default");
        lua_pushboolean(L, switch_node->has_default);
        lua_settable(L, -3);

        lua_pushstring(L, "case_count");
        lua_pushnumber(L, switch_node->case_count);
        lua_settable(L, -3);

        lua_settable(L, -3);
    }

    return 1;
}

int LuaBridge::lua_get_usings(lua_State* L) {
    if (!g_bridge || !g_bridge->current_ast_) {
        lua_newtable(L);
        return 1;
    }

    // using宣言を収集
    std::vector<std::shared_ptr<parser::UsingNode>> usings;
    std::function<void(std::shared_ptr<parser::ASTNode>, bool)> collect;
    collect = [&](std::shared_ptr<parser::ASTNode> node, bool in_scope) {
        if (!node)
            return;

        if (node->type == parser::ASTNodeType::Using) {
            auto using_node = std::static_pointer_cast<parser::UsingNode>(node);
            // グローバルスコープのusing namespaceのみを記録
            if (!in_scope && using_node->target.find("namespace") != std::string::npos) {
                usings.push_back(using_node);
            }
        }

        // namespace, class, functionの中に入ったらスコープ内とマーク
        bool child_in_scope = in_scope || node->type == parser::ASTNodeType::Namespace ||
                              node->type == parser::ASTNodeType::Class ||
                              node->type == parser::ASTNodeType::Function;

        for (auto& child : node->children) {
            collect(child, child_in_scope);
        }
    };

    collect(g_bridge->current_ast_, false);

    // Luaテーブルを作成
    lua_newtable(L);

    for (size_t i = 0; i < usings.size(); i++) {
        lua_pushnumber(L, i + 1);
        lua_newtable(L);

        auto using_node = usings[i];

        lua_pushstring(L, "line");
        lua_pushnumber(L, using_node->position.line);
        lua_settable(L, -3);

        lua_pushstring(L, "target");
        lua_pushstring(L, using_node->target.c_str());
        lua_settable(L, -3);

        lua_settable(L, -3);
    }

    return 1;
}

int LuaBridge::lua_get_includes(lua_State* L) {
    if (!g_bridge) {
        lua_newtable(L);
        return 1;
    }

    // 現在のファイルから#includeを抽出（テキストベース）
    std::ifstream file(g_bridge->current_file_);
    if (!file.is_open()) {
        lua_newtable(L);
        return 1;
    }

    lua_newtable(L);
    int index = 1;
    std::string line;
    int line_num = 0;

    while (std::getline(file, line)) {
        line_num++;
        // #include <...> or #include "..."
        if (line.find("#include") != std::string::npos) {
            lua_pushnumber(L, index++);
            lua_newtable(L);

            lua_pushstring(L, "line");
            lua_pushnumber(L, line_num);
            lua_settable(L, -3);

            lua_pushstring(L, "text");
            lua_pushstring(L, line.c_str());
            lua_settable(L, -3);

            // system include (<>) か local include ("") か判定
            bool is_system = line.find("<") != std::string::npos;
            lua_pushstring(L, "is_system");
            lua_pushboolean(L, is_system);
            lua_settable(L, -3);

            lua_settable(L, -3);
        }
    }

    return 1;
}

int LuaBridge::lua_get_file_info(lua_State* L) {
    if (!g_bridge) {
        lua_pushnil(L);
        return 1;
    }

    lua_newtable(L);

    // ファイルパス
    lua_pushstring(L, "path");
    lua_pushstring(L, g_bridge->current_file_.c_str());
    lua_settable(L, -3);

    // 行情報を返す
    std::ifstream file(g_bridge->current_file_);
    if (file.is_open()) {
        lua_pushstring(L, "lines");
        lua_newtable(L);

        std::string line;
        int line_num = 0;

        while (std::getline(file, line)) {
            line_num++;
            lua_pushnumber(L, line_num);
            lua_newtable(L);

            lua_pushstring(L, "text");
            lua_pushstring(L, line.c_str());
            lua_settable(L, -3);

            lua_pushstring(L, "length");
            lua_pushnumber(L, line.length());
            lua_settable(L, -3);

            // 行末の空白チェック
            bool has_trailing_space = !line.empty() && (line.back() == ' ' || line.back() == '\t');
            lua_pushstring(L, "has_trailing_space");
            lua_pushboolean(L, has_trailing_space);
            lua_settable(L, -3);

            // インデント情報
            size_t indent_count = 0;
            bool uses_tabs = false;
            for (char c : line) {
                if (c == ' ')
                    indent_count++;
                else if (c == '\t') {
                    uses_tabs = true;
                    indent_count++;
                } else
                    break;
            }

            lua_pushstring(L, "indent_count");
            lua_pushnumber(L, indent_count);
            lua_settable(L, -3);

            lua_pushstring(L, "uses_tabs");
            lua_pushboolean(L, uses_tabs);
            lua_settable(L, -3);

            // 空行チェック
            bool is_empty = line.find_first_not_of(" \t") == std::string::npos;
            lua_pushstring(L, "is_empty");
            lua_pushboolean(L, is_empty);
            lua_settable(L, -3);

            lua_settable(L, -3);
        }

        lua_settable(L, -3);
    }

    return 1;
}

int LuaBridge::lua_get_typedefs(lua_State* L) {
    if (!g_bridge || !g_bridge->current_ast_) {
        lua_newtable(L);
        return 1;
    }

    // typedefを収集
    std::vector<std::shared_ptr<parser::TypedefNode>> typedefs;
    std::function<void(std::shared_ptr<parser::ASTNode>)> collect;
    collect = [&](std::shared_ptr<parser::ASTNode> node) {
        if (!node)
            return;

        if (node->type == parser::ASTNodeType::Typedef) {
            typedefs.push_back(std::static_pointer_cast<parser::TypedefNode>(node));
        }

        for (auto& child : node->children) {
            collect(child);
        }
    };

    collect(g_bridge->current_ast_);

    // Luaテーブルを作成
    lua_newtable(L);

    for (size_t i = 0; i < typedefs.size(); i++) {
        lua_pushnumber(L, i + 1);
        lua_newtable(L);

        auto typedef_node = typedefs[i];

        lua_pushstring(L, "name");
        lua_pushstring(L, typedef_node->name.c_str());
        lua_settable(L, -3);

        lua_pushstring(L, "old_name");
        lua_pushstring(L, typedef_node->old_name.c_str());
        lua_settable(L, -3);

        lua_pushstring(L, "new_name");
        lua_pushstring(L, typedef_node->new_name.c_str());
        lua_settable(L, -3);

        lua_pushstring(L, "line");
        lua_pushnumber(L, typedef_node->position.line);
        lua_settable(L, -3);

        lua_settable(L, -3);
    }

    return 1;
}

int LuaBridge::lua_get_typedef_info(lua_State* L) {
    const char* typedef_name = luaL_checkstring(L, 1);

    if (!g_bridge || !g_bridge->current_ast_) {
        lua_pushnil(L);
        return 1;
    }

    // typedefを検索
    std::shared_ptr<parser::TypedefNode> found_typedef;
    std::function<void(std::shared_ptr<parser::ASTNode>)> find_typedef;
    find_typedef = [&](std::shared_ptr<parser::ASTNode> node) {
        if (!node || found_typedef)
            return;

        if (node->type == parser::ASTNodeType::Typedef) {
            auto typedef_node = std::dynamic_pointer_cast<parser::TypedefNode>(node);
            if (typedef_node &&
                (typedef_node->name == typedef_name || typedef_node->new_name == typedef_name)) {
                found_typedef = typedef_node;
                return;
            }
        }

        for (const auto& child : node->children) {
            find_typedef(child);
        }
    };

    find_typedef(g_bridge->current_ast_);

    if (!found_typedef) {
        lua_pushnil(L);
        return 1;
    }

    // typedef情報をテーブルで返す
    lua_newtable(L);

    lua_pushstring(L, "name");
    lua_pushstring(L, found_typedef->name.c_str());
    lua_settable(L, -3);

    lua_pushstring(L, "old_name");
    lua_pushstring(L, found_typedef->old_name.c_str());
    lua_settable(L, -3);

    lua_pushstring(L, "new_name");
    lua_pushstring(L, found_typedef->new_name.c_str());
    lua_settable(L, -3);

    lua_pushstring(L, "line");
    lua_pushnumber(L, found_typedef->position.line);
    lua_settable(L, -3);

    return 1;
}

int LuaBridge::lua_get_variables(lua_State* L) {
    if (!g_bridge || !g_bridge->current_ast_) {
        lua_newtable(L);
        return 1;
    }

    // グローバル変数を収集（TranslationUnitの直接の子のみ）
    std::vector<std::shared_ptr<parser::VariableNode>> variables;
    std::function<void(std::shared_ptr<parser::ASTNode>, bool)> collect;
    collect = [&](std::shared_ptr<parser::ASTNode> node, bool is_global) {
        if (!node)
            return;

        if (node->type == parser::ASTNodeType::Variable && is_global) {
            variables.push_back(std::static_pointer_cast<parser::VariableNode>(node));
        }

        // TranslationUnitとNamespaceの直接の子のみをグローバルとして扱う
        bool child_is_global = (node->type == parser::ASTNodeType::TranslationUnit) ||
                               (node->type == parser::ASTNodeType::Namespace && is_global);

        for (auto& child : node->children) {
            collect(child, child_is_global);
        }
    };

    collect(g_bridge->current_ast_, true);

    // Luaテーブルを作成
    lua_newtable(L);

    for (size_t i = 0; i < variables.size(); i++) {
        lua_pushnumber(L, i + 1);
        lua_newtable(L);

        auto var_node = variables[i];

        lua_pushstring(L, "name");
        lua_pushstring(L, var_node->name.c_str());
        lua_settable(L, -3);

        lua_pushstring(L, "type");
        lua_pushstring(L, var_node->type_name.c_str());
        lua_settable(L, -3);

        lua_pushstring(L, "is_const");
        lua_pushboolean(L, var_node->is_const);
        lua_settable(L, -3);

        lua_pushstring(L, "is_static");
        lua_pushboolean(L, var_node->is_static);
        lua_settable(L, -3);

        lua_pushstring(L, "is_constexpr");
        lua_pushboolean(L, var_node->is_constexpr);
        lua_settable(L, -3);

        lua_pushstring(L, "line");
        lua_pushnumber(L, var_node->position.line);
        lua_settable(L, -3);

        lua_settable(L, -3);
    }

    return 1;
}

int LuaBridge::lua_get_variable_info(lua_State* L) {
    const char* var_name = luaL_checkstring(L, 1);

    if (!g_bridge || !g_bridge->current_ast_) {
        lua_pushnil(L);
        return 1;
    }

    // 変数を検索
    std::shared_ptr<parser::VariableNode> found_var;
    std::function<void(std::shared_ptr<parser::ASTNode>)> find_var;
    find_var = [&](std::shared_ptr<parser::ASTNode> node) {
        if (!node || found_var)
            return;

        if (node->type == parser::ASTNodeType::Variable) {
            auto var_node = std::dynamic_pointer_cast<parser::VariableNode>(node);
            if (var_node && var_node->name == var_name) {
                found_var = var_node;
                return;
            }
        }

        for (const auto& child : node->children) {
            find_var(child);
        }
    };

    find_var(g_bridge->current_ast_);

    if (!found_var) {
        lua_pushnil(L);
        return 1;
    }

    // 変数情報をテーブルで返す
    lua_newtable(L);

    lua_pushstring(L, "name");
    lua_pushstring(L, found_var->name.c_str());
    lua_settable(L, -3);

    lua_pushstring(L, "type");
    lua_pushstring(L, found_var->type_name.c_str());
    lua_settable(L, -3);

    lua_pushstring(L, "is_const");
    lua_pushboolean(L, found_var->is_const);
    lua_settable(L, -3);

    lua_pushstring(L, "is_static");
    lua_pushboolean(L, found_var->is_static);
    lua_settable(L, -3);

    lua_pushstring(L, "is_constexpr");
    lua_pushboolean(L, found_var->is_constexpr);
    lua_settable(L, -3);

    lua_pushstring(L, "line");
    lua_pushnumber(L, found_var->position.line);
    lua_settable(L, -3);

    return 1;
}

int LuaBridge::lua_get_macros(lua_State* L) {
    if (!g_bridge || !g_bridge->current_ast_) {
        lua_newtable(L);
        return 1;
    }

    // マクロを収集
    std::vector<std::shared_ptr<parser::MacroNode>> macros;
    std::function<void(std::shared_ptr<parser::ASTNode>)> collect;
    collect = [&](std::shared_ptr<parser::ASTNode> node) {
        if (!node)
            return;

        if (node->type == parser::ASTNodeType::Macro) {
            macros.push_back(std::static_pointer_cast<parser::MacroNode>(node));
        }

        for (auto& child : node->children) {
            collect(child);
        }
    };

    collect(g_bridge->current_ast_);

    // Luaテーブルを作成
    lua_newtable(L);

    for (size_t i = 0; i < macros.size(); i++) {
        lua_pushnumber(L, i + 1);
        lua_newtable(L);

        auto macro_node = macros[i];

        lua_pushstring(L, "name");
        lua_pushstring(L, macro_node->name.c_str());
        lua_settable(L, -3);

        lua_pushstring(L, "is_function");
        lua_pushboolean(L, macro_node->is_function);
        lua_settable(L, -3);

        lua_pushstring(L, "definition");
        lua_pushstring(L, macro_node->definition.c_str());
        lua_settable(L, -3);

        lua_pushstring(L, "line");
        lua_pushnumber(L, macro_node->position.line);
        lua_settable(L, -3);

        // パラメータ配列
        lua_pushstring(L, "parameters");
        lua_newtable(L);
        for (size_t j = 0; j < macro_node->parameters.size(); j++) {
            lua_pushnumber(L, j + 1);
            lua_pushstring(L, macro_node->parameters[j].c_str());
            lua_settable(L, -3);
        }
        lua_settable(L, -3);

        lua_settable(L, -3);
    }

    return 1;
}

int LuaBridge::lua_get_macro_info(lua_State* L) {
    const char* macro_name = luaL_checkstring(L, 1);

    if (!g_bridge || !g_bridge->current_ast_) {
        lua_pushnil(L);
        return 1;
    }

    // マクロを検索
    std::shared_ptr<parser::MacroNode> found_macro;
    std::function<void(std::shared_ptr<parser::ASTNode>)> find_macro;
    find_macro = [&](std::shared_ptr<parser::ASTNode> node) {
        if (!node || found_macro)
            return;

        if (node->type == parser::ASTNodeType::Macro) {
            auto macro_node = std::dynamic_pointer_cast<parser::MacroNode>(node);
            if (macro_node && macro_node->name == macro_name) {
                found_macro = macro_node;
                return;
            }
        }

        for (const auto& child : node->children) {
            find_macro(child);
        }
    };

    find_macro(g_bridge->current_ast_);

    if (!found_macro) {
        lua_pushnil(L);
        return 1;
    }

    // マクロ情報をテーブルで返す
    lua_newtable(L);

    lua_pushstring(L, "name");
    lua_pushstring(L, found_macro->name.c_str());
    lua_settable(L, -3);

    lua_pushstring(L, "is_function");
    lua_pushboolean(L, found_macro->is_function);
    lua_settable(L, -3);

    lua_pushstring(L, "definition");
    lua_pushstring(L, found_macro->definition.c_str());
    lua_settable(L, -3);

    lua_pushstring(L, "line");
    lua_pushnumber(L, found_macro->position.line);
    lua_settable(L, -3);

    // パラメータ配列
    lua_pushstring(L, "parameters");
    lua_newtable(L);
    for (size_t i = 0; i < found_macro->parameters.size(); i++) {
        lua_pushnumber(L, i + 1);
        lua_pushstring(L, found_macro->parameters[i].c_str());
        lua_settable(L, -3);
    }
    lua_settable(L, -3);

    return 1;
}

int LuaBridge::lua_get_if_statements(lua_State* L) {
    if (!g_bridge || !g_bridge->current_ast_) {
        lua_newtable(L);
        return 1;
    }

    // if文を収集
    std::vector<std::shared_ptr<parser::IfStatementNode>> if_stmts;
    std::function<void(std::shared_ptr<parser::ASTNode>)> collect;
    collect = [&](std::shared_ptr<parser::ASTNode> node) {
        if (!node)
            return;

        if (node->type == parser::ASTNodeType::IfStatement) {
            if_stmts.push_back(std::static_pointer_cast<parser::IfStatementNode>(node));
        }

        for (auto& child : node->children) {
            collect(child);
        }
    };

    collect(g_bridge->current_ast_);

    // Luaテーブルを作成
    lua_newtable(L);

    for (size_t i = 0; i < if_stmts.size(); i++) {
        lua_pushnumber(L, i + 1);
        lua_newtable(L);

        auto if_node = if_stmts[i];

        lua_pushstring(L, "line");
        lua_pushnumber(L, if_node->position.line);
        lua_settable(L, -3);

        lua_pushstring(L, "has_braces");
        lua_pushboolean(L, if_node->has_braces);
        lua_settable(L, -3);

        lua_pushstring(L, "has_else");
        lua_pushboolean(L, if_node->has_else);
        lua_settable(L, -3);

        lua_settable(L, -3);
    }

    return 1;
}

int LuaBridge::lua_get_loops(lua_State* L) {
    if (!g_bridge || !g_bridge->current_ast_) {
        lua_newtable(L);
        return 1;
    }

    // ループを収集
    std::vector<std::shared_ptr<parser::LoopStatementNode>> loops;
    std::function<void(std::shared_ptr<parser::ASTNode>)> collect;
    collect = [&](std::shared_ptr<parser::ASTNode> node) {
        if (!node)
            return;

        if (node->type == parser::ASTNodeType::LoopStatement) {
            loops.push_back(std::static_pointer_cast<parser::LoopStatementNode>(node));
        }

        for (auto& child : node->children) {
            collect(child);
        }
    };

    collect(g_bridge->current_ast_);

    // Luaテーブルを作成
    lua_newtable(L);

    for (size_t i = 0; i < loops.size(); i++) {
        lua_pushnumber(L, i + 1);
        lua_newtable(L);

        auto loop_node = loops[i];

        lua_pushstring(L, "line");
        lua_pushnumber(L, loop_node->position.line);
        lua_settable(L, -3);

        lua_pushstring(L, "has_braces");
        lua_pushboolean(L, loop_node->has_braces);
        lua_settable(L, -3);

        // loop_typeを文字列に変換
        const char* loop_type_str = "unknown";
        switch (loop_node->loop_type) {
            case parser::LoopStatementNode::LoopType::For:
                loop_type_str = "for";
                break;
            case parser::LoopStatementNode::LoopType::While:
                loop_type_str = "while";
                break;
            case parser::LoopStatementNode::LoopType::DoWhile:
                loop_type_str = "do_while";
                break;
        }

        lua_pushstring(L, "type");
        lua_pushstring(L, loop_type_str);
        lua_settable(L, -3);

        lua_settable(L, -3);
    }

    return 1;
}

int LuaBridge::lua_get_try_statements(lua_State* L) {
    if (!g_bridge || !g_bridge->current_ast_) {
        lua_newtable(L);
        return 1;
    }

    // try文を収集
    std::vector<std::shared_ptr<parser::TryStatementNode>> try_stmts;
    std::function<void(std::shared_ptr<parser::ASTNode>)> collect;
    collect = [&](std::shared_ptr<parser::ASTNode> node) {
        if (!node)
            return;

        if (node->type == parser::ASTNodeType::TryStatement) {
            try_stmts.push_back(std::static_pointer_cast<parser::TryStatementNode>(node));
        }

        for (auto& child : node->children) {
            collect(child);
        }
    };

    collect(g_bridge->current_ast_);

    // Luaテーブルを作成
    lua_newtable(L);

    for (size_t i = 0; i < try_stmts.size(); i++) {
        lua_pushnumber(L, i + 1);
        lua_newtable(L);

        auto try_node = try_stmts[i];

        lua_pushstring(L, "line");
        lua_pushnumber(L, try_node->position.line);
        lua_settable(L, -3);

        lua_pushstring(L, "catch_count");
        lua_pushnumber(L, try_node->catch_count);
        lua_settable(L, -3);

        lua_pushstring(L, "has_finally");
        lua_pushboolean(L, try_node->has_finally);
        lua_settable(L, -3);

        lua_settable(L, -3);
    }

    return 1;
}

int LuaBridge::lua_get_comments(lua_State* L) {
    if (!g_bridge || !g_bridge->current_ast_) {
        lua_newtable(L);
        return 1;
    }

    // コメントを収集
    std::vector<std::shared_ptr<parser::CommentNode>> comments;
    std::function<void(std::shared_ptr<parser::ASTNode>)> collect;
    collect = [&](std::shared_ptr<parser::ASTNode> node) {
        if (!node)
            return;

        if (node->type == parser::ASTNodeType::Comment) {
            comments.push_back(std::static_pointer_cast<parser::CommentNode>(node));
        }

        for (auto& child : node->children) {
            collect(child);
        }
    };

    collect(g_bridge->current_ast_);

    // Luaテーブルを作成
    lua_newtable(L);

    for (size_t i = 0; i < comments.size(); i++) {
        lua_pushnumber(L, i + 1);
        lua_newtable(L);

        auto comment_node = comments[i];

        lua_pushstring(L, "line");
        lua_pushnumber(L, comment_node->position.line);
        lua_settable(L, -3);

        lua_pushstring(L, "content");
        lua_pushstring(L, comment_node->content.c_str());
        lua_settable(L, -3);

        lua_pushstring(L, "is_line_comment");
        lua_pushboolean(L, comment_node->is_line_comment);
        lua_settable(L, -3);

        lua_settable(L, -3);
    }

    return 1;
}

int LuaBridge::lua_get_constructors(lua_State* L) {
    if (!g_bridge || !g_bridge->current_ast_) {
        lua_newtable(L);
        return 1;
    }

    // コンストラクタを収集
    std::vector<std::shared_ptr<parser::ConstructorNode>> constructors;
    std::function<void(std::shared_ptr<parser::ASTNode>)> collect;
    collect = [&](std::shared_ptr<parser::ASTNode> node) {
        if (!node)
            return;

        if (node->type == parser::ASTNodeType::Constructor) {
            constructors.push_back(std::static_pointer_cast<parser::ConstructorNode>(node));
        }

        for (auto& child : node->children) {
            collect(child);
        }
    };

    collect(g_bridge->current_ast_);

    // Luaテーブルを作成
    lua_newtable(L);

    for (size_t i = 0; i < constructors.size(); i++) {
        lua_pushnumber(L, i + 1);
        lua_newtable(L);

        auto ctor = constructors[i];

        lua_pushstring(L, "class_name");
        lua_pushstring(L, ctor->class_name.c_str());
        lua_settable(L, -3);

        lua_pushstring(L, "line");
        lua_pushnumber(L, ctor->position.line);
        lua_settable(L, -3);

        lua_pushstring(L, "is_default");
        lua_pushboolean(L, ctor->is_default);
        lua_settable(L, -3);

        lua_pushstring(L, "is_delete");
        lua_pushboolean(L, ctor->is_delete);
        lua_settable(L, -3);

        lua_pushstring(L, "is_explicit");
        lua_pushboolean(L, ctor->is_explicit);
        lua_settable(L, -3);

        lua_pushstring(L, "access");
        lua_pushnumber(L, static_cast<int>(ctor->access));
        lua_settable(L, -3);

        lua_settable(L, -3);
    }

    return 1;
}

int LuaBridge::lua_get_constructor_info(lua_State* L) {
    const char* class_name = luaL_checkstring(L, 1);

    if (!g_bridge || !g_bridge->current_ast_) {
        lua_pushnil(L);
        return 1;
    }

    // コンストラクタを検索
    std::shared_ptr<parser::ConstructorNode> found_ctor;
    std::function<void(std::shared_ptr<parser::ASTNode>)> find_ctor;
    find_ctor = [&](std::shared_ptr<parser::ASTNode> node) {
        if (!node || found_ctor)
            return;

        if (node->type == parser::ASTNodeType::Constructor) {
            auto ctor = std::dynamic_pointer_cast<parser::ConstructorNode>(node);
            if (ctor && ctor->class_name == class_name) {
                found_ctor = ctor;
                return;
            }
        }

        for (const auto& child : node->children) {
            find_ctor(child);
        }
    };

    find_ctor(g_bridge->current_ast_);

    if (!found_ctor) {
        lua_pushnil(L);
        return 1;
    }

    // コンストラクタ情報をテーブルで返す
    lua_newtable(L);

    lua_pushstring(L, "class_name");
    lua_pushstring(L, found_ctor->class_name.c_str());
    lua_settable(L, -3);

    lua_pushstring(L, "line");
    lua_pushnumber(L, found_ctor->position.line);
    lua_settable(L, -3);

    lua_pushstring(L, "is_default");
    lua_pushboolean(L, found_ctor->is_default);
    lua_settable(L, -3);

    lua_pushstring(L, "is_delete");
    lua_pushboolean(L, found_ctor->is_delete);
    lua_settable(L, -3);

    lua_pushstring(L, "is_explicit");
    lua_pushboolean(L, found_ctor->is_explicit);
    lua_settable(L, -3);

    lua_pushstring(L, "is_constexpr");
    lua_pushboolean(L, found_ctor->is_constexpr);
    lua_settable(L, -3);

    lua_pushstring(L, "access");
    lua_pushnumber(L, static_cast<int>(found_ctor->access));
    lua_settable(L, -3);

    return 1;
}

int LuaBridge::lua_get_destructors(lua_State* L) {
    if (!g_bridge || !g_bridge->current_ast_) {
        lua_newtable(L);
        return 1;
    }

    // デストラクタを収集
    std::vector<std::shared_ptr<parser::DestructorNode>> destructors;
    std::function<void(std::shared_ptr<parser::ASTNode>)> collect;
    collect = [&](std::shared_ptr<parser::ASTNode> node) {
        if (!node)
            return;

        if (node->type == parser::ASTNodeType::Destructor) {
            destructors.push_back(std::static_pointer_cast<parser::DestructorNode>(node));
        }

        for (auto& child : node->children) {
            collect(child);
        }
    };

    collect(g_bridge->current_ast_);

    // Luaテーブルを作成
    lua_newtable(L);

    for (size_t i = 0; i < destructors.size(); i++) {
        lua_pushnumber(L, i + 1);
        lua_newtable(L);

        auto dtor = destructors[i];

        lua_pushstring(L, "class_name");
        lua_pushstring(L, dtor->class_name.c_str());
        lua_settable(L, -3);

        lua_pushstring(L, "line");
        lua_pushnumber(L, dtor->position.line);
        lua_settable(L, -3);

        lua_pushstring(L, "is_virtual");
        lua_pushboolean(L, dtor->is_virtual);
        lua_settable(L, -3);

        lua_pushstring(L, "is_default");
        lua_pushboolean(L, dtor->is_default);
        lua_settable(L, -3);

        lua_pushstring(L, "is_delete");
        lua_pushboolean(L, dtor->is_delete);
        lua_settable(L, -3);

        lua_settable(L, -3);
    }

    return 1;
}

int LuaBridge::lua_get_destructor_info(lua_State* L) {
    const char* class_name = luaL_checkstring(L, 1);

    if (!g_bridge || !g_bridge->current_ast_) {
        lua_pushnil(L);
        return 1;
    }

    // デストラクタを検索
    std::shared_ptr<parser::DestructorNode> found_dtor;
    std::function<void(std::shared_ptr<parser::ASTNode>)> find_dtor;
    find_dtor = [&](std::shared_ptr<parser::ASTNode> node) {
        if (!node || found_dtor)
            return;

        if (node->type == parser::ASTNodeType::Destructor) {
            auto dtor = std::dynamic_pointer_cast<parser::DestructorNode>(node);
            if (dtor && dtor->class_name == class_name) {
                found_dtor = dtor;
                return;
            }
        }

        for (const auto& child : node->children) {
            find_dtor(child);
        }
    };

    find_dtor(g_bridge->current_ast_);

    if (!found_dtor) {
        lua_pushnil(L);
        return 1;
    }

    // デストラクタ情報をテーブルで返す
    lua_newtable(L);

    lua_pushstring(L, "class_name");
    lua_pushstring(L, found_dtor->class_name.c_str());
    lua_settable(L, -3);

    lua_pushstring(L, "line");
    lua_pushnumber(L, found_dtor->position.line);
    lua_settable(L, -3);

    lua_pushstring(L, "is_virtual");
    lua_pushboolean(L, found_dtor->is_virtual);
    lua_settable(L, -3);

    lua_pushstring(L, "is_default");
    lua_pushboolean(L, found_dtor->is_default);
    lua_settable(L, -3);

    lua_pushstring(L, "is_delete");
    lua_pushboolean(L, found_dtor->is_delete);
    lua_settable(L, -3);

    lua_pushstring(L, "is_noexcept");
    lua_pushboolean(L, found_dtor->is_noexcept);
    lua_settable(L, -3);

    return 1;
}

int LuaBridge::lua_get_operators(lua_State* L) {
    if (!g_bridge || !g_bridge->current_ast_) {
        lua_newtable(L);
        return 1;
    }

    // 演算子オーバーロードを収集
    std::vector<std::shared_ptr<parser::OperatorNode>> operators;
    std::function<void(std::shared_ptr<parser::ASTNode>)> collect;
    collect = [&](std::shared_ptr<parser::ASTNode> node) {
        if (!node)
            return;

        if (node->type == parser::ASTNodeType::Operator) {
            operators.push_back(std::static_pointer_cast<parser::OperatorNode>(node));
        }

        for (auto& child : node->children) {
            collect(child);
        }
    };

    collect(g_bridge->current_ast_);

    // Luaテーブルを作成
    lua_newtable(L);

    for (size_t i = 0; i < operators.size(); i++) {
        lua_pushnumber(L, i + 1);
        lua_newtable(L);

        auto op = operators[i];

        lua_pushstring(L, "operator");
        lua_pushstring(L, op->operator_symbol.c_str());
        lua_settable(L, -3);

        lua_pushstring(L, "line");
        lua_pushnumber(L, op->position.line);
        lua_settable(L, -3);

        lua_pushstring(L, "is_member");
        lua_pushboolean(L, op->is_member);
        lua_settable(L, -3);

        lua_pushstring(L, "is_friend");
        lua_pushboolean(L, op->is_friend);
        lua_settable(L, -3);

        lua_settable(L, -3);
    }

    return 1;
}

int LuaBridge::lua_get_operator_info(lua_State* L) {
    const char* op_symbol = luaL_checkstring(L, 1);

    if (!g_bridge || !g_bridge->current_ast_) {
        lua_pushnil(L);
        return 1;
    }

    // 演算子を検索
    std::shared_ptr<parser::OperatorNode> found_op;
    std::function<void(std::shared_ptr<parser::ASTNode>)> find_op;
    find_op = [&](std::shared_ptr<parser::ASTNode> node) {
        if (!node || found_op)
            return;

        if (node->type == parser::ASTNodeType::Operator) {
            auto op = std::dynamic_pointer_cast<parser::OperatorNode>(node);
            if (op && op->operator_symbol == op_symbol) {
                found_op = op;
                return;
            }
        }

        for (const auto& child : node->children) {
            find_op(child);
        }
    };

    find_op(g_bridge->current_ast_);

    if (!found_op) {
        lua_pushnil(L);
        return 1;
    }

    // 演算子情報をテーブルで返す
    lua_newtable(L);

    lua_pushstring(L, "operator");
    lua_pushstring(L, found_op->operator_symbol.c_str());
    lua_settable(L, -3);

    lua_pushstring(L, "return_type");
    lua_pushstring(L, found_op->return_type.c_str());
    lua_settable(L, -3);

    lua_pushstring(L, "line");
    lua_pushnumber(L, found_op->position.line);
    lua_settable(L, -3);

    lua_pushstring(L, "is_member");
    lua_pushboolean(L, found_op->is_member);
    lua_settable(L, -3);

    lua_pushstring(L, "is_friend");
    lua_pushboolean(L, found_op->is_friend);
    lua_settable(L, -3);

    return 1;
}

int LuaBridge::lua_get_templates(lua_State* L) {
    if (!g_bridge || !g_bridge->current_ast_) {
        lua_newtable(L);
        return 1;
    }

    // テンプレートを収集
    std::vector<std::shared_ptr<parser::TemplateNode>> templates;
    std::function<void(std::shared_ptr<parser::ASTNode>)> collect;
    collect = [&](std::shared_ptr<parser::ASTNode> node) {
        if (!node)
            return;

        if (node->type == parser::ASTNodeType::Template) {
            templates.push_back(std::static_pointer_cast<parser::TemplateNode>(node));
        }

        for (auto& child : node->children) {
            collect(child);
        }
    };

    collect(g_bridge->current_ast_);

    // Luaテーブルを作成
    lua_newtable(L);

    for (size_t i = 0; i < templates.size(); i++) {
        lua_pushnumber(L, i + 1);
        lua_newtable(L);

        auto tmpl = templates[i];

        lua_pushstring(L, "name");
        lua_pushstring(L, tmpl->name.c_str());
        lua_settable(L, -3);

        lua_pushstring(L, "line");
        lua_pushnumber(L, tmpl->position.line);
        lua_settable(L, -3);

        lua_pushstring(L, "is_variadic");
        lua_pushboolean(L, tmpl->is_variadic);
        lua_settable(L, -3);

        lua_pushstring(L, "is_specialization");
        lua_pushboolean(L, tmpl->is_specialization);
        lua_settable(L, -3);

        lua_settable(L, -3);
    }

    return 1;
}

int LuaBridge::lua_get_template_info(lua_State* L) {
    const char* template_name = luaL_checkstring(L, 1);

    if (!g_bridge || !g_bridge->current_ast_) {
        lua_pushnil(L);
        return 1;
    }

    // テンプレートを検索
    std::shared_ptr<parser::TemplateNode> found_tmpl;
    std::function<void(std::shared_ptr<parser::ASTNode>)> find_tmpl;
    find_tmpl = [&](std::shared_ptr<parser::ASTNode> node) {
        if (!node || found_tmpl)
            return;

        if (node->type == parser::ASTNodeType::Template) {
            auto tmpl = std::dynamic_pointer_cast<parser::TemplateNode>(node);
            if (tmpl && tmpl->name == template_name) {
                found_tmpl = tmpl;
                return;
            }
        }

        for (const auto& child : node->children) {
            find_tmpl(child);
        }
    };

    find_tmpl(g_bridge->current_ast_);

    if (!found_tmpl) {
        lua_pushnil(L);
        return 1;
    }

    // テンプレート情報をテーブルで返す
    lua_newtable(L);

    lua_pushstring(L, "name");
    lua_pushstring(L, found_tmpl->name.c_str());
    lua_settable(L, -3);

    lua_pushstring(L, "line");
    lua_pushnumber(L, found_tmpl->position.line);
    lua_settable(L, -3);

    lua_pushstring(L, "is_variadic");
    lua_pushboolean(L, found_tmpl->is_variadic);
    lua_settable(L, -3);

    lua_pushstring(L, "is_specialization");
    lua_pushboolean(L, found_tmpl->is_specialization);
    lua_settable(L, -3);

    lua_pushstring(L, "is_partial_specialization");
    lua_pushboolean(L, found_tmpl->is_partial_specialization);
    lua_settable(L, -3);

    return 1;
}

int LuaBridge::lua_get_lambdas(lua_State* L) {
    if (!g_bridge || !g_bridge->current_ast_) {
        lua_newtable(L);
        return 1;
    }

    // ラムダ式を収集
    std::vector<std::shared_ptr<parser::LambdaNode>> lambdas;
    std::function<void(std::shared_ptr<parser::ASTNode>)> collect;
    collect = [&](std::shared_ptr<parser::ASTNode> node) {
        if (!node)
            return;

        if (node->type == parser::ASTNodeType::Lambda) {
            lambdas.push_back(std::static_pointer_cast<parser::LambdaNode>(node));
        }

        for (auto& child : node->children) {
            collect(child);
        }
    };

    collect(g_bridge->current_ast_);

    // Luaテーブルを作成
    lua_newtable(L);

    for (size_t i = 0; i < lambdas.size(); i++) {
        lua_pushnumber(L, i + 1);
        lua_newtable(L);

        auto lambda = lambdas[i];

        lua_pushstring(L, "line");
        lua_pushnumber(L, lambda->position.line);
        lua_settable(L, -3);

        lua_pushstring(L, "capture_clause");
        lua_pushstring(L, lambda->capture_clause.c_str());
        lua_settable(L, -3);

        lua_pushstring(L, "is_mutable");
        lua_pushboolean(L, lambda->is_mutable);
        lua_settable(L, -3);

        lua_settable(L, -3);
    }

    return 1;
}

int LuaBridge::lua_get_lambda_info(lua_State* L) {
    int line_num = luaL_checkinteger(L, 1);

    if (!g_bridge || !g_bridge->current_ast_) {
        lua_pushnil(L);
        return 1;
    }

    // ラムダを検索
    std::shared_ptr<parser::LambdaNode> found_lambda;
    std::function<void(std::shared_ptr<parser::ASTNode>)> find_lambda;
    find_lambda = [&](std::shared_ptr<parser::ASTNode> node) {
        if (!node || found_lambda)
            return;

        if (node->type == parser::ASTNodeType::Lambda) {
            auto lambda = std::dynamic_pointer_cast<parser::LambdaNode>(node);
            if (lambda && lambda->position.line == line_num) {
                found_lambda = lambda;
                return;
            }
        }

        for (const auto& child : node->children) {
            find_lambda(child);
        }
    };

    find_lambda(g_bridge->current_ast_);

    if (!found_lambda) {
        lua_pushnil(L);
        return 1;
    }

    // ラムダ情報をテーブルで返す
    lua_newtable(L);

    lua_pushstring(L, "line");
    lua_pushnumber(L, found_lambda->position.line);
    lua_settable(L, -3);

    lua_pushstring(L, "capture_clause");
    lua_pushstring(L, found_lambda->capture_clause.c_str());
    lua_settable(L, -3);

    lua_pushstring(L, "return_type");
    lua_pushstring(L, found_lambda->return_type.c_str());
    lua_settable(L, -3);

    lua_pushstring(L, "is_mutable");
    lua_pushboolean(L, found_lambda->is_mutable);
    lua_settable(L, -3);

    lua_pushstring(L, "is_constexpr");
    lua_pushboolean(L, found_lambda->is_constexpr);
    lua_settable(L, -3);

    return 1;
}

int LuaBridge::lua_get_friends(lua_State* L) {
    if (!g_bridge || !g_bridge->current_ast_) {
        lua_newtable(L);
        return 1;
    }

    // friend宣言を収集
    std::vector<std::shared_ptr<parser::FriendNode>> friends;
    std::function<void(std::shared_ptr<parser::ASTNode>)> collect;
    collect = [&](std::shared_ptr<parser::ASTNode> node) {
        if (!node)
            return;

        if (node->type == parser::ASTNodeType::Friend) {
            friends.push_back(std::static_pointer_cast<parser::FriendNode>(node));
        }

        for (auto& child : node->children) {
            collect(child);
        }
    };

    collect(g_bridge->current_ast_);

    // Luaテーブルを作成
    lua_newtable(L);

    for (size_t i = 0; i < friends.size(); i++) {
        lua_pushnumber(L, i + 1);
        lua_newtable(L);

        auto fr = friends[i];

        lua_pushstring(L, "target");
        lua_pushstring(L, fr->target_name.c_str());
        lua_settable(L, -3);

        lua_pushstring(L, "line");
        lua_pushnumber(L, fr->position.line);
        lua_settable(L, -3);

        lua_pushstring(L, "kind");
        lua_pushstring(L, fr->kind == parser::FriendNode::FriendKind::Class ? "class" : "function");
        lua_settable(L, -3);

        lua_settable(L, -3);
    }

    return 1;
}

int LuaBridge::lua_get_static_asserts(lua_State* L) {
    if (!g_bridge || !g_bridge->current_ast_) {
        lua_newtable(L);
        return 1;
    }

    // static_assertを収集
    std::vector<std::shared_ptr<parser::StaticAssertNode>> static_asserts;
    std::function<void(std::shared_ptr<parser::ASTNode>)> collect;
    collect = [&](std::shared_ptr<parser::ASTNode> node) {
        if (!node)
            return;

        if (node->type == parser::ASTNodeType::StaticAssert) {
            static_asserts.push_back(std::static_pointer_cast<parser::StaticAssertNode>(node));
        }

        for (auto& child : node->children) {
            collect(child);
        }
    };

    collect(g_bridge->current_ast_);

    // Luaテーブルを作成
    lua_newtable(L);

    for (size_t i = 0; i < static_asserts.size(); i++) {
        lua_pushnumber(L, i + 1);
        lua_newtable(L);

        auto sa = static_asserts[i];

        lua_pushstring(L, "condition");
        lua_pushstring(L, sa->condition.c_str());
        lua_settable(L, -3);

        lua_pushstring(L, "message");
        lua_pushstring(L, sa->message.c_str());
        lua_settable(L, -3);

        lua_pushstring(L, "line");
        lua_pushnumber(L, sa->position.line);
        lua_settable(L, -3);

        lua_settable(L, -3);
    }

    return 1;
}

int LuaBridge::lua_get_call_graph(lua_State* L) {
    if (!g_bridge || !g_bridge->current_ast_) {
        lua_newtable(L);
        return 1;
    }

    // 関数呼び出しグラフを構築
    std::map<std::string, std::vector<std::string>> call_graph;

    std::vector<std::shared_ptr<parser::CallExpressionNode>> calls;
    std::function<void(std::shared_ptr<parser::ASTNode>)> collect;
    collect = [&](std::shared_ptr<parser::ASTNode> node) {
        if (!node)
            return;

        if (node->type == parser::ASTNodeType::CallExpression) {
            auto call = std::static_pointer_cast<parser::CallExpressionNode>(node);
            if (!call->caller_function.empty() && !call->function_name.empty()) {
                call_graph[call->caller_function].push_back(call->function_name);
            }
        }

        for (auto& child : node->children) {
            collect(child);
        }
    };

    collect(g_bridge->current_ast_);

    // Luaテーブルを作成
    lua_newtable(L);

    for (const auto& pair : call_graph) {
        lua_pushstring(L, pair.first.c_str());
        lua_newtable(L);

        for (size_t i = 0; i < pair.second.size(); i++) {
            lua_pushnumber(L, i + 1);
            lua_pushstring(L, pair.second[i].c_str());
            lua_settable(L, -3);
        }

        lua_settable(L, -3);
    }

    return 1;
}

int LuaBridge::lua_get_function_calls(lua_State* L) {
    if (!g_bridge || !g_bridge->current_ast_) {
        lua_newtable(L);
        return 1;
    }

    // 関数呼び出しを収集
    std::vector<std::shared_ptr<parser::CallExpressionNode>> calls;
    std::function<void(std::shared_ptr<parser::ASTNode>)> collect;
    collect = [&](std::shared_ptr<parser::ASTNode> node) {
        if (!node)
            return;

        if (node->type == parser::ASTNodeType::CallExpression) {
            calls.push_back(std::static_pointer_cast<parser::CallExpressionNode>(node));
        }

        for (auto& child : node->children) {
            collect(child);
        }
    };

    collect(g_bridge->current_ast_);

    // Luaテーブルを作成
    lua_newtable(L);

    for (size_t i = 0; i < calls.size(); i++) {
        lua_pushnumber(L, i + 1);
        lua_newtable(L);

        auto call = calls[i];

        lua_pushstring(L, "function");
        lua_pushstring(L, call->function_name.c_str());
        lua_settable(L, -3);

        lua_pushstring(L, "caller");
        lua_pushstring(L, call->caller_function.c_str());
        lua_settable(L, -3);

        lua_pushstring(L, "line");
        lua_pushnumber(L, call->position.line);
        lua_settable(L, -3);

        lua_pushstring(L, "scope");
        lua_pushstring(L, call->scope.c_str());
        lua_settable(L, -3);

        lua_settable(L, -3);
    }

    return 1;
}

int LuaBridge::lua_get_callers(lua_State* L) {
    const char* function_name = luaL_checkstring(L, 1);

    if (!g_bridge || !g_bridge->current_ast_) {
        lua_newtable(L);
        return 1;
    }

    // この関数を呼び出している関数を検索
    std::vector<std::string> callers;
    std::function<void(std::shared_ptr<parser::ASTNode>)> collect;
    collect = [&](std::shared_ptr<parser::ASTNode> node) {
        if (!node)
            return;

        if (node->type == parser::ASTNodeType::CallExpression) {
            auto call = std::static_pointer_cast<parser::CallExpressionNode>(node);
            if (call->function_name == function_name && !call->caller_function.empty()) {
                callers.push_back(call->caller_function);
            }
        }

        for (auto& child : node->children) {
            collect(child);
        }
    };

    collect(g_bridge->current_ast_);

    // Luaテーブルを作成
    lua_newtable(L);

    for (size_t i = 0; i < callers.size(); i++) {
        lua_pushnumber(L, i + 1);
        lua_pushstring(L, callers[i].c_str());
        lua_settable(L, -3);
    }

    return 1;
}

int LuaBridge::lua_get_callees(lua_State* L) {
    const char* function_name = luaL_checkstring(L, 1);

    if (!g_bridge || !g_bridge->current_ast_) {
        lua_newtable(L);
        return 1;
    }

    // この関数が呼び出している関数を検索
    std::vector<std::string> callees;
    std::function<void(std::shared_ptr<parser::ASTNode>)> collect;
    collect = [&](std::shared_ptr<parser::ASTNode> node) {
        if (!node)
            return;

        if (node->type == parser::ASTNodeType::CallExpression) {
            auto call = std::static_pointer_cast<parser::CallExpressionNode>(node);
            if (call->caller_function == function_name) {
                callees.push_back(call->function_name);
            }
        }

        for (auto& child : node->children) {
            collect(child);
        }
    };

    collect(g_bridge->current_ast_);

    // Luaテーブルを作成
    lua_newtable(L);

    for (size_t i = 0; i < callees.size(); i++) {
        lua_pushnumber(L, i + 1);
        lua_pushstring(L, callees[i].c_str());
        lua_settable(L, -3);
    }

    return 1;
}

int LuaBridge::lua_get_return_statements(lua_State* L) {
    if (!g_bridge || !g_bridge->current_ast_) {
        lua_newtable(L);
        return 1;
    }

    // return文を収集
    std::vector<std::shared_ptr<parser::ReturnStatementNode>> returns;
    std::function<void(std::shared_ptr<parser::ASTNode>)> collect;
    collect = [&](std::shared_ptr<parser::ASTNode> node) {
        if (!node)
            return;

        if (node->type == parser::ASTNodeType::ReturnStatement) {
            returns.push_back(std::static_pointer_cast<parser::ReturnStatementNode>(node));
        }

        for (auto& child : node->children) {
            collect(child);
        }
    };

    collect(g_bridge->current_ast_);

    // Luaテーブルを作成
    lua_newtable(L);

    for (size_t i = 0; i < returns.size(); i++) {
        lua_pushnumber(L, i + 1);
        lua_newtable(L);

        auto ret = returns[i];

        lua_pushstring(L, "line");
        lua_pushnumber(L, ret->position.line);
        lua_settable(L, -3);

        lua_pushstring(L, "has_value");
        lua_pushboolean(L, ret->has_value);
        lua_settable(L, -3);

        lua_pushstring(L, "return_value");
        lua_pushstring(L, ret->return_value.c_str());
        lua_settable(L, -3);

        lua_settable(L, -3);
    }

    return 1;
}

int LuaBridge::lua_get_inheritance_tree(lua_State* L) {
    if (!g_bridge || !g_bridge->current_ast_) {
        lua_newtable(L);
        return 1;
    }

    // クラスの継承ツリーを構築
    std::map<std::string, std::vector<std::string>> inheritance_tree;

    std::vector<std::shared_ptr<parser::ClassNode>> classes;
    std::function<void(std::shared_ptr<parser::ASTNode>)> collect;
    collect = [&](std::shared_ptr<parser::ASTNode> node) {
        if (!node)
            return;

        if (node->type == parser::ASTNodeType::Class) {
            classes.push_back(std::static_pointer_cast<parser::ClassNode>(node));
        }

        for (auto& child : node->children) {
            collect(child);
        }
    };

    collect(g_bridge->current_ast_);

    // 継承関係を記録
    for (const auto& cls : classes) {
        std::vector<std::string> bases;
        for (const auto& base : cls->base_classes) {
            bases.push_back(base.base_class_name);
        }
        inheritance_tree[cls->name] = bases;
    }

    // Luaテーブルを作成
    lua_newtable(L);

    for (const auto& pair : inheritance_tree) {
        lua_pushstring(L, pair.first.c_str());
        lua_newtable(L);

        for (size_t i = 0; i < pair.second.size(); i++) {
            lua_pushnumber(L, i + 1);
            lua_pushstring(L, pair.second[i].c_str());
            lua_settable(L, -3);
        }

        lua_settable(L, -3);
    }

    return 1;
}

int LuaBridge::lua_get_attributes(lua_State* L) {
    if (!g_bridge || !g_bridge->current_ast_) {
        lua_newtable(L);
        return 1;
    }

    // 属性を収集（すべてのノードから）
    std::map<int, std::vector<std::string>> attributes_by_line;

    std::function<void(std::shared_ptr<parser::ASTNode>)> collect;
    collect = [&](std::shared_ptr<parser::ASTNode> node) {
        if (!node)
            return;

        // FunctionNode, ClassNode, VariableNode などから属性を収集
        if (node->type == parser::ASTNodeType::Function) {
            auto func = std::static_pointer_cast<parser::FunctionNode>(node);
            for (const auto& attr : func->attributes) {
                attributes_by_line[func->position.line].push_back(attr.name);
            }
        } else if (node->type == parser::ASTNodeType::Class) {
            auto cls = std::static_pointer_cast<parser::ClassNode>(node);
            for (const auto& attr : cls->attributes) {
                attributes_by_line[cls->position.line].push_back(attr.name);
            }
        }

        for (auto& child : node->children) {
            collect(child);
        }
    };

    collect(g_bridge->current_ast_);

    // Luaテーブルを作成
    lua_newtable(L);

    for (const auto& pair : attributes_by_line) {
        lua_pushnumber(L, pair.first);
        lua_newtable(L);

        for (size_t i = 0; i < pair.second.size(); i++) {
            lua_pushnumber(L, i + 1);
            lua_pushstring(L, pair.second[i].c_str());
            lua_settable(L, -3);
        }

        lua_settable(L, -3);
    }

    return 1;
}

// ========== Enhanced AST APIs for complex linting rules ==========

int LuaBridge::lua_get_classes_with_info(lua_State* L) {
    if (!g_bridge || !g_bridge->current_ast_) {
        lua_newtable(L);
        return 1;
    }

    lua_newtable(L);
    int index = 1;

    // 親namespaceを追跡しながらクラスを収集
    std::function<void(std::shared_ptr<parser::ASTNode>, const std::string&)> collect_classes;
    collect_classes = [&](std::shared_ptr<parser::ASTNode> node,
                          const std::string& current_namespace) {
        if (!node)
            return;

        std::string ns = current_namespace;
        if (node->type == parser::ASTNodeType::Namespace) {
            auto ns_node = std::dynamic_pointer_cast<parser::NamespaceNode>(node);
            if (ns_node) {
                ns = current_namespace.empty() ? ns_node->name
                                               : current_namespace + "::" + ns_node->name;
            }
        }

        if (node->type == parser::ASTNodeType::Class) {
            auto class_node = std::dynamic_pointer_cast<parser::ClassNode>(node);
            if (class_node) {
                lua_pushinteger(L, index++);
                lua_newtable(L);

                lua_pushstring(L, "name");
                lua_pushstring(L, class_node->name.c_str());
                lua_settable(L, -3);

                lua_pushstring(L, "namespace");
                lua_pushstring(L, ns.c_str());
                lua_settable(L, -3);

                lua_pushstring(L, "qualified_name");
                std::string qualified =
                    ns.empty() ? class_node->name : ns + "::" + class_node->name;
                lua_pushstring(L, qualified.c_str());
                lua_settable(L, -3);

                lua_pushstring(L, "line");
                lua_pushinteger(L, class_node->position.line);
                lua_settable(L, -3);

                lua_pushstring(L, "is_struct");
                lua_pushboolean(L, class_node->is_struct);
                lua_settable(L, -3);

                lua_pushstring(L, "is_abstract");
                lua_pushboolean(L, class_node->is_abstract);
                lua_settable(L, -3);

                lua_pushstring(L, "is_final");
                lua_pushboolean(L, class_node->is_final);
                lua_settable(L, -3);

                lua_pushstring(L, "is_template");
                lua_pushboolean(L, class_node->is_template);
                lua_settable(L, -3);

                // Base classes
                lua_pushstring(L, "base_classes");
                lua_newtable(L);
                for (size_t i = 0; i < class_node->base_classes.size(); i++) {
                    lua_pushinteger(L, i + 1);
                    lua_newtable(L);

                    lua_pushstring(L, "name");
                    lua_pushstring(L, class_node->base_classes[i].base_class_name.c_str());
                    lua_settable(L, -3);

                    const char* access_str = "public";
                    switch (class_node->base_classes[i].access) {
                        case parser::AccessSpecifier::Protected:
                            access_str = "protected";
                            break;
                        case parser::AccessSpecifier::Private:
                            access_str = "private";
                            break;
                        default:
                            break;
                    }
                    lua_pushstring(L, "access");
                    lua_pushstring(L, access_str);
                    lua_settable(L, -3);

                    lua_pushstring(L, "is_virtual");
                    lua_pushboolean(L, class_node->base_classes[i].is_virtual);
                    lua_settable(L, -3);

                    lua_settable(L, -3);
                }
                lua_settable(L, -3);

                lua_settable(L, -3);
            }
        }

        for (const auto& child : node->children) {
            collect_classes(child, ns);
        }
    };

    collect_classes(g_bridge->current_ast_, "");
    return 1;
}

int LuaBridge::lua_get_methods_with_info(lua_State* L) {
    const char* class_name = luaL_checkstring(L, 1);

    if (!g_bridge || !g_bridge->current_ast_) {
        lua_newtable(L);
        return 1;
    }

    // クラスを検索
    std::shared_ptr<parser::ClassNode> found_class;
    std::function<void(std::shared_ptr<parser::ASTNode>)> find_class;
    find_class = [&](std::shared_ptr<parser::ASTNode> node) {
        if (!node || found_class)
            return;

        if (node->type == parser::ASTNodeType::Class) {
            auto class_node = std::dynamic_pointer_cast<parser::ClassNode>(node);
            if (class_node && class_node->name == class_name) {
                found_class = class_node;
                return;
            }
        }

        for (const auto& child : node->children) {
            find_class(child);
        }
    };

    find_class(g_bridge->current_ast_);

    if (!found_class) {
        lua_newtable(L);
        return 1;
    }

    lua_newtable(L);
    int index = 1;

    for (const auto& child : found_class->children) {
        if (child->type == parser::ASTNodeType::Function ||
            child->type == parser::ASTNodeType::Method) {
            auto func = std::dynamic_pointer_cast<parser::FunctionNode>(child);
            if (func) {
                lua_pushinteger(L, index++);
                lua_newtable(L);

                lua_pushstring(L, "name");
                lua_pushstring(L, func->name.c_str());
                lua_settable(L, -3);

                lua_pushstring(L, "return_type");
                lua_pushstring(L, func->return_type.c_str());
                lua_settable(L, -3);

                lua_pushstring(L, "line");
                lua_pushinteger(L, func->position.line);
                lua_settable(L, -3);

                lua_pushstring(L, "column");
                lua_pushinteger(L, func->position.column);
                lua_settable(L, -3);

                // Access specifier
                const char* access_str = "none";
                switch (func->access) {
                    case parser::AccessSpecifier::Public:
                        access_str = "public";
                        break;
                    case parser::AccessSpecifier::Protected:
                        access_str = "protected";
                        break;
                    case parser::AccessSpecifier::Private:
                        access_str = "private";
                        break;
                    default:
                        break;
                }
                lua_pushstring(L, "access");
                lua_pushstring(L, access_str);
                lua_settable(L, -3);

                // Modifiers
                lua_pushstring(L, "is_const");
                lua_pushboolean(L, func->is_const);
                lua_settable(L, -3);

                lua_pushstring(L, "is_static");
                lua_pushboolean(L, func->is_static);
                lua_settable(L, -3);

                lua_pushstring(L, "is_virtual");
                lua_pushboolean(L, func->is_virtual);
                lua_settable(L, -3);

                lua_pushstring(L, "is_override");
                lua_pushboolean(L, func->is_override);
                lua_settable(L, -3);

                lua_pushstring(L, "is_final");
                lua_pushboolean(L, func->is_final);
                lua_settable(L, -3);

                lua_pushstring(L, "is_pure_virtual");
                lua_pushboolean(L, func->is_pure_virtual);
                lua_settable(L, -3);

                lua_pushstring(L, "is_noexcept");
                lua_pushboolean(L, func->is_noexcept);
                lua_settable(L, -3);

                lua_pushstring(L, "is_constexpr");
                lua_pushboolean(L, func->is_constexpr);
                lua_settable(L, -3);

                // Parameters
                lua_pushstring(L, "parameters");
                lua_newtable(L);
                size_t param_count =
                    std::min(func->parameter_types.size(), func->parameter_names.size());
                for (size_t i = 0; i < param_count; i++) {
                    lua_pushinteger(L, i + 1);
                    lua_newtable(L);

                    lua_pushstring(L, "type");
                    lua_pushstring(L, func->parameter_types[i].c_str());
                    lua_settable(L, -3);

                    lua_pushstring(L, "name");
                    lua_pushstring(L, func->parameter_names[i].c_str());
                    lua_settable(L, -3);

                    lua_settable(L, -3);
                }
                lua_settable(L, -3);

                lua_settable(L, -3);
            }
        }
    }

    return 1;
}

int LuaBridge::lua_get_all_methods(lua_State* L) {
    if (!g_bridge || !g_bridge->current_ast_) {
        lua_newtable(L);
        return 1;
    }

    lua_newtable(L);
    int index = 1;

    // すべてのクラスのメソッドを収集
    std::function<void(std::shared_ptr<parser::ASTNode>, const std::string&, const std::string&)>
        collect;
    collect = [&](std::shared_ptr<parser::ASTNode> node, const std::string& current_namespace,
                  const std::string& current_class) {
        if (!node)
            return;

        std::string ns = current_namespace;
        std::string cls = current_class;

        if (node->type == parser::ASTNodeType::Namespace) {
            auto ns_node = std::dynamic_pointer_cast<parser::NamespaceNode>(node);
            if (ns_node) {
                ns = current_namespace.empty() ? ns_node->name
                                               : current_namespace + "::" + ns_node->name;
            }
        } else if (node->type == parser::ASTNodeType::Class) {
            auto class_node = std::dynamic_pointer_cast<parser::ClassNode>(node);
            if (class_node) {
                cls = class_node->name;
            }
        }

        if ((node->type == parser::ASTNodeType::Function ||
             node->type == parser::ASTNodeType::Method) &&
            !cls.empty()) {
            auto func = std::dynamic_pointer_cast<parser::FunctionNode>(node);
            if (func) {
                lua_pushinteger(L, index++);
                lua_newtable(L);

                lua_pushstring(L, "name");
                lua_pushstring(L, func->name.c_str());
                lua_settable(L, -3);

                lua_pushstring(L, "class_name");
                lua_pushstring(L, cls.c_str());
                lua_settable(L, -3);

                lua_pushstring(L, "namespace");
                lua_pushstring(L, ns.c_str());
                lua_settable(L, -3);

                lua_pushstring(L, "return_type");
                lua_pushstring(L, func->return_type.c_str());
                lua_settable(L, -3);

                lua_pushstring(L, "line");
                lua_pushinteger(L, func->position.line);
                lua_settable(L, -3);

                const char* access_str = "none";
                switch (func->access) {
                    case parser::AccessSpecifier::Public:
                        access_str = "public";
                        break;
                    case parser::AccessSpecifier::Protected:
                        access_str = "protected";
                        break;
                    case parser::AccessSpecifier::Private:
                        access_str = "private";
                        break;
                    default:
                        break;
                }
                lua_pushstring(L, "access");
                lua_pushstring(L, access_str);
                lua_settable(L, -3);

                lua_pushstring(L, "is_const");
                lua_pushboolean(L, func->is_const);
                lua_settable(L, -3);

                lua_pushstring(L, "is_static");
                lua_pushboolean(L, func->is_static);
                lua_settable(L, -3);

                lua_pushstring(L, "is_virtual");
                lua_pushboolean(L, func->is_virtual);
                lua_settable(L, -3);

                // Parameters
                lua_pushstring(L, "parameters");
                lua_newtable(L);
                size_t param_count =
                    std::min(func->parameter_types.size(), func->parameter_names.size());
                for (size_t i = 0; i < param_count; i++) {
                    lua_pushinteger(L, i + 1);
                    lua_newtable(L);
                    lua_pushstring(L, "type");
                    lua_pushstring(L, func->parameter_types[i].c_str());
                    lua_settable(L, -3);
                    lua_pushstring(L, "name");
                    lua_pushstring(L, func->parameter_names[i].c_str());
                    lua_settable(L, -3);
                    lua_settable(L, -3);
                }
                lua_settable(L, -3);

                lua_settable(L, -3);
            }
        }

        for (const auto& child : node->children) {
            collect(child, ns, (node->type == parser::ASTNodeType::Class) ? cls : current_class);
        }
    };

    collect(g_bridge->current_ast_, "", "");
    return 1;
}

int LuaBridge::lua_get_function_info(lua_State* L) {
    const char* func_name = luaL_checkstring(L, 1);

    if (!g_bridge || !g_bridge->current_ast_) {
        lua_pushnil(L);
        return 1;
    }

    // 関数を検索
    std::shared_ptr<parser::FunctionNode> found_func;
    std::string found_namespace;

    std::function<void(std::shared_ptr<parser::ASTNode>, const std::string&)> find_func;
    find_func = [&](std::shared_ptr<parser::ASTNode> node, const std::string& ns) {
        if (!node || found_func)
            return;

        std::string current_ns = ns;
        if (node->type == parser::ASTNodeType::Namespace) {
            auto ns_node = std::dynamic_pointer_cast<parser::NamespaceNode>(node);
            if (ns_node) {
                current_ns = ns.empty() ? ns_node->name : ns + "::" + ns_node->name;
            }
        }

        if (node->type == parser::ASTNodeType::Function) {
            auto func = std::dynamic_pointer_cast<parser::FunctionNode>(node);
            if (func && func->name == func_name) {
                found_func = func;
                found_namespace = current_ns;
                return;
            }
        }

        for (const auto& child : node->children) {
            find_func(child, current_ns);
        }
    };

    find_func(g_bridge->current_ast_, "");

    if (!found_func) {
        lua_pushnil(L);
        return 1;
    }

    lua_newtable(L);

    lua_pushstring(L, "name");
    lua_pushstring(L, found_func->name.c_str());
    lua_settable(L, -3);

    lua_pushstring(L, "namespace");
    lua_pushstring(L, found_namespace.c_str());
    lua_settable(L, -3);

    lua_pushstring(L, "return_type");
    lua_pushstring(L, found_func->return_type.c_str());
    lua_settable(L, -3);

    lua_pushstring(L, "line");
    lua_pushinteger(L, found_func->position.line);
    lua_settable(L, -3);

    lua_pushstring(L, "is_static");
    lua_pushboolean(L, found_func->is_static);
    lua_settable(L, -3);

    lua_pushstring(L, "is_inline");
    lua_pushboolean(L, found_func->is_inline);
    lua_settable(L, -3);

    lua_pushstring(L, "is_constexpr");
    lua_pushboolean(L, found_func->is_constexpr);
    lua_settable(L, -3);

    lua_pushstring(L, "is_noexcept");
    lua_pushboolean(L, found_func->is_noexcept);
    lua_settable(L, -3);

    // Parameters
    lua_pushstring(L, "parameters");
    lua_newtable(L);
    size_t param_count =
        std::min(found_func->parameter_types.size(), found_func->parameter_names.size());
    for (size_t i = 0; i < param_count; i++) {
        lua_pushinteger(L, i + 1);
        lua_newtable(L);
        lua_pushstring(L, "type");
        lua_pushstring(L, found_func->parameter_types[i].c_str());
        lua_settable(L, -3);
        lua_pushstring(L, "name");
        lua_pushstring(L, found_func->parameter_names[i].c_str());
        lua_settable(L, -3);
        lua_settable(L, -3);
    }
    lua_settable(L, -3);

    return 1;
}

int LuaBridge::lua_get_function_parameters(lua_State* L) {
    const char* class_name = luaL_checkstring(L, 1);
    const char* method_name = luaL_checkstring(L, 2);

    if (!g_bridge || !g_bridge->current_ast_) {
        lua_newtable(L);
        return 1;
    }

    // クラスを検索
    std::shared_ptr<parser::ClassNode> found_class;
    std::function<void(std::shared_ptr<parser::ASTNode>)> find_class;
    find_class = [&](std::shared_ptr<parser::ASTNode> node) {
        if (!node || found_class)
            return;

        if (node->type == parser::ASTNodeType::Class) {
            auto class_node = std::dynamic_pointer_cast<parser::ClassNode>(node);
            if (class_node && class_node->name == class_name) {
                found_class = class_node;
                return;
            }
        }

        for (const auto& child : node->children) {
            find_class(child);
        }
    };

    find_class(g_bridge->current_ast_);

    if (!found_class) {
        lua_newtable(L);
        return 1;
    }

    // メソッドを検索
    std::shared_ptr<parser::FunctionNode> found_method;
    for (const auto& child : found_class->children) {
        if (child->type == parser::ASTNodeType::Function ||
            child->type == parser::ASTNodeType::Method) {
            auto func = std::dynamic_pointer_cast<parser::FunctionNode>(child);
            if (func && func->name == method_name) {
                found_method = func;
                break;
            }
        }
    }

    if (!found_method) {
        lua_newtable(L);
        return 1;
    }

    lua_newtable(L);
    size_t param_count =
        std::min(found_method->parameter_types.size(), found_method->parameter_names.size());
    for (size_t i = 0; i < param_count; i++) {
        lua_pushinteger(L, i + 1);
        lua_newtable(L);

        lua_pushstring(L, "type");
        lua_pushstring(L, found_method->parameter_types[i].c_str());
        lua_settable(L, -3);

        lua_pushstring(L, "name");
        lua_pushstring(L, found_method->parameter_names[i].c_str());
        lua_settable(L, -3);

        lua_settable(L, -3);
    }

    return 1;
}

int LuaBridge::lua_get_classes_in_namespace(lua_State* L) {
    const char* namespace_name = luaL_checkstring(L, 1);

    if (!g_bridge || !g_bridge->current_ast_) {
        lua_newtable(L);
        return 1;
    }

    lua_newtable(L);
    int index = 1;

    // 指定されたnamespace内のクラスを収集
    std::function<void(std::shared_ptr<parser::ASTNode>, const std::string&)> collect;
    collect = [&](std::shared_ptr<parser::ASTNode> node, const std::string& current_ns) {
        if (!node)
            return;

        std::string ns = current_ns;
        if (node->type == parser::ASTNodeType::Namespace) {
            auto ns_node = std::dynamic_pointer_cast<parser::NamespaceNode>(node);
            if (ns_node) {
                ns = current_ns.empty() ? ns_node->name : current_ns + "::" + ns_node->name;
            }
        }

        if (node->type == parser::ASTNodeType::Class && ns == namespace_name) {
            auto class_node = std::dynamic_pointer_cast<parser::ClassNode>(node);
            if (class_node) {
                lua_pushinteger(L, index++);
                lua_newtable(L);

                lua_pushstring(L, "name");
                lua_pushstring(L, class_node->name.c_str());
                lua_settable(L, -3);

                lua_pushstring(L, "line");
                lua_pushinteger(L, class_node->position.line);
                lua_settable(L, -3);

                lua_pushstring(L, "is_struct");
                lua_pushboolean(L, class_node->is_struct);
                lua_settable(L, -3);

                lua_settable(L, -3);
            }
        }

        for (const auto& child : node->children) {
            collect(child, ns);
        }
    };

    collect(g_bridge->current_ast_, "");
    return 1;
}

int LuaBridge::lua_get_functions_in_namespace(lua_State* L) {
    const char* namespace_name = luaL_checkstring(L, 1);

    if (!g_bridge || !g_bridge->current_ast_) {
        lua_newtable(L);
        return 1;
    }

    lua_newtable(L);
    int index = 1;

    // 指定されたnamespace内の関数を収集（クラスメソッドは除く）
    std::function<void(std::shared_ptr<parser::ASTNode>, const std::string&, bool)> collect;
    collect = [&](std::shared_ptr<parser::ASTNode> node, const std::string& current_ns,
                  bool in_class) {
        if (!node)
            return;

        std::string ns = current_ns;
        bool is_in_class = in_class;

        if (node->type == parser::ASTNodeType::Namespace) {
            auto ns_node = std::dynamic_pointer_cast<parser::NamespaceNode>(node);
            if (ns_node) {
                ns = current_ns.empty() ? ns_node->name : current_ns + "::" + ns_node->name;
            }
        } else if (node->type == parser::ASTNodeType::Class) {
            is_in_class = true;
        }

        if (node->type == parser::ASTNodeType::Function && !is_in_class && ns == namespace_name) {
            auto func = std::dynamic_pointer_cast<parser::FunctionNode>(node);
            if (func) {
                lua_pushinteger(L, index++);
                lua_newtable(L);

                lua_pushstring(L, "name");
                lua_pushstring(L, func->name.c_str());
                lua_settable(L, -3);

                lua_pushstring(L, "return_type");
                lua_pushstring(L, func->return_type.c_str());
                lua_settable(L, -3);

                lua_pushstring(L, "line");
                lua_pushinteger(L, func->position.line);
                lua_settable(L, -3);

                lua_settable(L, -3);
            }
        }

        for (const auto& child : node->children) {
            collect(child, ns, is_in_class);
        }
    };

    collect(g_bridge->current_ast_, "", false);
    return 1;
}

int LuaBridge::lua_get_include_details(lua_State* L) {
    if (!g_bridge) {
        lua_newtable(L);
        return 1;
    }

    std::ifstream file(g_bridge->current_file_);
    if (!file.is_open()) {
        lua_newtable(L);
        return 1;
    }

    lua_newtable(L);
    int index = 1;
    std::string line;
    int line_num = 0;

    while (std::getline(file, line)) {
        line_num++;
        // #include <...> or #include "..."
        size_t include_pos = line.find("#include");
        if (include_pos != std::string::npos) {
            lua_pushinteger(L, index++);
            lua_newtable(L);

            lua_pushstring(L, "line");
            lua_pushinteger(L, line_num);
            lua_settable(L, -3);

            lua_pushstring(L, "text");
            lua_pushstring(L, line.c_str());
            lua_settable(L, -3);

            // Parse header name
            std::string header_name;
            bool is_system = false;

            size_t angle_start = line.find('<');
            size_t angle_end = line.find('>');
            size_t quote_start = line.find('"');
            size_t quote_end = line.rfind('"');

            if (angle_start != std::string::npos && angle_end != std::string::npos &&
                angle_start < angle_end) {
                header_name = line.substr(angle_start + 1, angle_end - angle_start - 1);
                is_system = true;
            } else if (quote_start != std::string::npos && quote_end != std::string::npos &&
                       quote_start < quote_end) {
                header_name = line.substr(quote_start + 1, quote_end - quote_start - 1);
                is_system = false;
            }

            lua_pushstring(L, "header");
            lua_pushstring(L, header_name.c_str());
            lua_settable(L, -3);

            lua_pushstring(L, "is_system");
            lua_pushboolean(L, is_system);
            lua_settable(L, -3);

            lua_settable(L, -3);
        }
    }

    return 1;
}

int LuaBridge::lua_get_namespace_info(lua_State* L) {
    const char* namespace_name = luaL_checkstring(L, 1);

    if (!g_bridge || !g_bridge->current_ast_) {
        lua_pushnil(L);
        return 1;
    }

    // namespaceを検索
    std::shared_ptr<parser::NamespaceNode> found_ns;
    std::function<void(std::shared_ptr<parser::ASTNode>)> find_ns;
    find_ns = [&](std::shared_ptr<parser::ASTNode> node) {
        if (!node || found_ns)
            return;

        if (node->type == parser::ASTNodeType::Namespace) {
            auto ns_node = std::dynamic_pointer_cast<parser::NamespaceNode>(node);
            if (ns_node && ns_node->name == namespace_name) {
                found_ns = ns_node;
                return;
            }
        }

        for (const auto& child : node->children) {
            find_ns(child);
        }
    };

    find_ns(g_bridge->current_ast_);

    if (!found_ns) {
        lua_pushnil(L);
        return 1;
    }

    lua_newtable(L);

    lua_pushstring(L, "name");
    lua_pushstring(L, found_ns->name.c_str());
    lua_settable(L, -3);

    lua_pushstring(L, "line");
    lua_pushinteger(L, found_ns->position.line);
    lua_settable(L, -3);

    // Count classes
    int class_count = 0;
    int func_count = 0;
    for (const auto& child : found_ns->children) {
        if (child->type == parser::ASTNodeType::Class)
            class_count++;
        if (child->type == parser::ASTNodeType::Function)
            func_count++;
    }

    lua_pushstring(L, "class_count");
    lua_pushinteger(L, class_count);
    lua_settable(L, -3);

    lua_pushstring(L, "function_count");
    lua_pushinteger(L, func_count);
    lua_settable(L, -3);

    return 1;
}

int LuaBridge::lua_get_class_methods_by_access(lua_State* L) {
    const char* class_name = luaL_checkstring(L, 1);
    const char* access_filter = luaL_checkstring(L, 2);  // "public", "protected", "private"

    if (!g_bridge || !g_bridge->current_ast_) {
        lua_newtable(L);
        return 1;
    }

    parser::AccessSpecifier target_access = parser::AccessSpecifier::None;
    if (strcmp(access_filter, "public") == 0)
        target_access = parser::AccessSpecifier::Public;
    else if (strcmp(access_filter, "protected") == 0)
        target_access = parser::AccessSpecifier::Protected;
    else if (strcmp(access_filter, "private") == 0)
        target_access = parser::AccessSpecifier::Private;

    // クラスを検索
    std::shared_ptr<parser::ClassNode> found_class;
    std::function<void(std::shared_ptr<parser::ASTNode>)> find_class;
    find_class = [&](std::shared_ptr<parser::ASTNode> node) {
        if (!node || found_class)
            return;

        if (node->type == parser::ASTNodeType::Class) {
            auto class_node = std::dynamic_pointer_cast<parser::ClassNode>(node);
            if (class_node && class_node->name == class_name) {
                found_class = class_node;
                return;
            }
        }

        for (const auto& child : node->children) {
            find_class(child);
        }
    };

    find_class(g_bridge->current_ast_);

    if (!found_class) {
        lua_newtable(L);
        return 1;
    }

    lua_newtable(L);
    int index = 1;

    for (const auto& child : found_class->children) {
        if (child->type == parser::ASTNodeType::Function ||
            child->type == parser::ASTNodeType::Method) {
            auto func = std::dynamic_pointer_cast<parser::FunctionNode>(child);
            if (func && func->access == target_access) {
                lua_pushinteger(L, index++);
                lua_newtable(L);

                lua_pushstring(L, "name");
                lua_pushstring(L, func->name.c_str());
                lua_settable(L, -3);

                lua_pushstring(L, "return_type");
                lua_pushstring(L, func->return_type.c_str());
                lua_settable(L, -3);

                lua_pushstring(L, "line");
                lua_pushinteger(L, func->position.line);
                lua_settable(L, -3);

                lua_pushstring(L, "is_const");
                lua_pushboolean(L, func->is_const);
                lua_settable(L, -3);

                lua_pushstring(L, "is_static");
                lua_pushboolean(L, func->is_static);
                lua_settable(L, -3);

                lua_pushstring(L, "is_virtual");
                lua_pushboolean(L, func->is_virtual);
                lua_settable(L, -3);

                // Parameters
                lua_pushstring(L, "parameters");
                lua_newtable(L);
                size_t param_count =
                    std::min(func->parameter_types.size(), func->parameter_names.size());
                for (size_t i = 0; i < param_count; i++) {
                    lua_pushinteger(L, i + 1);
                    lua_newtable(L);
                    lua_pushstring(L, "type");
                    lua_pushstring(L, func->parameter_types[i].c_str());
                    lua_settable(L, -3);
                    lua_pushstring(L, "name");
                    lua_pushstring(L, func->parameter_names[i].c_str());
                    lua_settable(L, -3);
                    lua_settable(L, -3);
                }
                lua_settable(L, -3);

                lua_settable(L, -3);
            }
        }
    }

    return 1;
}

int LuaBridge::lua_get_class_fields_by_access(lua_State* L) {
    const char* class_name = luaL_checkstring(L, 1);
    const char* access_filter = luaL_checkstring(L, 2);

    if (!g_bridge || !g_bridge->current_ast_) {
        lua_newtable(L);
        return 1;
    }

    parser::AccessSpecifier target_access = parser::AccessSpecifier::None;
    if (strcmp(access_filter, "public") == 0)
        target_access = parser::AccessSpecifier::Public;
    else if (strcmp(access_filter, "protected") == 0)
        target_access = parser::AccessSpecifier::Protected;
    else if (strcmp(access_filter, "private") == 0)
        target_access = parser::AccessSpecifier::Private;

    // クラスを検索
    std::shared_ptr<parser::ClassNode> found_class;
    std::function<void(std::shared_ptr<parser::ASTNode>)> find_class;
    find_class = [&](std::shared_ptr<parser::ASTNode> node) {
        if (!node || found_class)
            return;

        if (node->type == parser::ASTNodeType::Class) {
            auto class_node = std::dynamic_pointer_cast<parser::ClassNode>(node);
            if (class_node && class_node->name == class_name) {
                found_class = class_node;
                return;
            }
        }

        for (const auto& child : node->children) {
            find_class(child);
        }
    };

    find_class(g_bridge->current_ast_);

    if (!found_class) {
        lua_newtable(L);
        return 1;
    }

    lua_newtable(L);
    int index = 1;

    for (const auto& child : found_class->children) {
        if (child->type == parser::ASTNodeType::Field) {
            auto field = std::dynamic_pointer_cast<parser::FieldNode>(child);
            if (field && field->access == target_access) {
                lua_pushinteger(L, index++);
                lua_newtable(L);

                lua_pushstring(L, "name");
                lua_pushstring(L, field->name.c_str());
                lua_settable(L, -3);

                lua_pushstring(L, "type");
                lua_pushstring(L, field->type_name.c_str());
                lua_settable(L, -3);

                lua_pushstring(L, "line");
                lua_pushinteger(L, field->position.line);
                lua_settable(L, -3);

                lua_pushstring(L, "is_const");
                lua_pushboolean(L, field->is_const);
                lua_settable(L, -3);

                lua_pushstring(L, "is_static");
                lua_pushboolean(L, field->is_static);
                lua_settable(L, -3);

                lua_pushstring(L, "is_mutable");
                lua_pushboolean(L, field->is_mutable);
                lua_settable(L, -3);

                lua_settable(L, -3);
            }
        }
    }

    return 1;
}

int LuaBridge::lua_is_call_allowed(lua_State* L) {
    const char* caller_func = luaL_checkstring(L, 1);
    const char* called_func = luaL_checkstring(L, 2);

    if (!g_bridge || !g_bridge->current_ast_) {
        lua_pushboolean(L, 1);  // 呼び出しを許可（デフォルト）
        return 1;
    }

    // caller_funcからcalled_funcへの呼び出しが存在するか確認
    bool call_exists = false;
    std::function<void(std::shared_ptr<parser::ASTNode>)> check_calls;
    check_calls = [&](std::shared_ptr<parser::ASTNode> node) {
        if (!node || call_exists)
            return;

        if (node->type == parser::ASTNodeType::CallExpression) {
            auto call = std::static_pointer_cast<parser::CallExpressionNode>(node);
            if (call->caller_function == caller_func && call->function_name == called_func) {
                call_exists = true;
                return;
            }
        }

        for (auto& child : node->children) {
            check_calls(child);
        }
    };

    check_calls(g_bridge->current_ast_);

    lua_pushboolean(L, call_exists ? 1 : 0);
    return 1;
}

int LuaBridge::lua_get_all_using_declarations(lua_State* L) {
    if (!g_bridge || !g_bridge->current_ast_) {
        lua_newtable(L);
        return 1;
    }

    lua_newtable(L);
    int index = 1;

    std::function<void(std::shared_ptr<parser::ASTNode>, const std::string&, bool)> collect;
    collect = [&](std::shared_ptr<parser::ASTNode> node, const std::string& scope, bool is_global) {
        if (!node)
            return;

        std::string current_scope = scope;
        bool in_global = is_global;

        if (node->type == parser::ASTNodeType::Namespace) {
            auto ns = std::dynamic_pointer_cast<parser::NamespaceNode>(node);
            if (ns) {
                current_scope = scope.empty() ? ns->name : scope + "::" + ns->name;
                in_global = false;
            }
        } else if (node->type == parser::ASTNodeType::Class ||
                   node->type == parser::ASTNodeType::Function) {
            in_global = false;
        }

        if (node->type == parser::ASTNodeType::Using) {
            auto using_node = std::static_pointer_cast<parser::UsingNode>(node);
            lua_pushinteger(L, index++);
            lua_newtable(L);

            lua_pushstring(L, "line");
            lua_pushinteger(L, using_node->position.line);
            lua_settable(L, -3);

            lua_pushstring(L, "target");
            lua_pushstring(L, using_node->target.c_str());
            lua_settable(L, -3);

            lua_pushstring(L, "alias");
            lua_pushstring(L, using_node->alias.c_str());
            lua_settable(L, -3);

            lua_pushstring(L, "scope");
            lua_pushstring(L, current_scope.c_str());
            lua_settable(L, -3);

            lua_pushstring(L, "is_global");
            lua_pushboolean(L, in_global);
            lua_settable(L, -3);

            const char* kind_str = "namespace";
            if (using_node->kind == parser::UsingNode::UsingKind::TypeAlias)
                kind_str = "type_alias";
            else if (using_node->kind == parser::UsingNode::UsingKind::Declaration)
                kind_str = "declaration";
            lua_pushstring(L, "kind");
            lua_pushstring(L, kind_str);
            lua_settable(L, -3);

            lua_settable(L, -3);
        }

        for (auto& child : node->children) {
            collect(child, current_scope, in_global);
        }
    };

    collect(g_bridge->current_ast_, "", true);
    return 1;
}

int LuaBridge::lua_get_base_classes(lua_State* L) {
    const char* class_name = luaL_checkstring(L, 1);

    if (!g_bridge || !g_bridge->current_ast_) {
        lua_newtable(L);
        return 1;
    }

    // クラスを検索
    std::shared_ptr<parser::ClassNode> found_class;
    std::function<void(std::shared_ptr<parser::ASTNode>)> find_class;
    find_class = [&](std::shared_ptr<parser::ASTNode> node) {
        if (!node || found_class)
            return;

        if (node->type == parser::ASTNodeType::Class) {
            auto class_node = std::dynamic_pointer_cast<parser::ClassNode>(node);
            if (class_node && class_node->name == class_name) {
                found_class = class_node;
                return;
            }
        }

        for (const auto& child : node->children) {
            find_class(child);
        }
    };

    find_class(g_bridge->current_ast_);

    if (!found_class) {
        lua_newtable(L);
        return 1;
    }

    lua_newtable(L);
    int index = 1;

    for (const auto& base : found_class->base_classes) {
        lua_pushinteger(L, index++);
        lua_newtable(L);

        lua_pushstring(L, "name");
        lua_pushstring(L, base.base_class_name.c_str());
        lua_settable(L, -3);

        const char* access_str = "public";
        switch (base.access) {
            case parser::AccessSpecifier::Protected:
                access_str = "protected";
                break;
            case parser::AccessSpecifier::Private:
                access_str = "private";
                break;
            default:
                break;
        }
        lua_pushstring(L, "access");
        lua_pushstring(L, access_str);
        lua_settable(L, -3);

        lua_pushstring(L, "is_virtual");
        lua_pushboolean(L, base.is_virtual);
        lua_settable(L, -3);

        lua_settable(L, -3);
    }

    return 1;
}

#else  // HAVE_LUAJIT が定義されていない場合（スタブ実装）

LuaBridge::LuaBridge(std::shared_ptr<LuaEngine> lua_engine) : lua_engine_(lua_engine) {}

void LuaBridge::set_diagnostic_engine(diagnostic::DiagnosticEngine* diag_engine) {
    (void)diag_engine;
}

void LuaBridge::set_current_file(const std::string& file_path) {
    (void)file_path;
}

void LuaBridge::set_current_ast(std::shared_ptr<parser::TranslationUnitNode> ast) {
    (void)ast;
}

void LuaBridge::register_api() {}

bool LuaBridge::call_function(const std::string& function_name,
                              const std::vector<std::string>& args) {
    (void)function_name;
    (void)args;
    return false;
}

void LuaBridge::report_diagnostic(const std::string& file_path, int line, int column,
                                  const std::string& message, diagnostic::Severity severity) {
    (void)file_path;
    (void)line;
    (void)column;
    (void)message;
    (void)severity;
}

int LuaBridge::lua_report_error(lua_State* L) {
    (void)L;
    return 0;
}
int LuaBridge::lua_report_warning(lua_State* L) {
    (void)L;
    return 0;
}
int LuaBridge::lua_report_info(lua_State* L) {
    (void)L;
    return 0;
}
int LuaBridge::lua_get_file_content(lua_State* L) {
    (void)L;
    return 0;
}
int LuaBridge::lua_match_pattern(lua_State* L) {
    (void)L;
    return 0;
}
int LuaBridge::lua_get_classes(lua_State* L) {
    (void)L;
    return 0;
}
int LuaBridge::lua_get_class_info(lua_State* L) {
    (void)L;
    return 0;
}
int LuaBridge::lua_get_methods(lua_State* L) {
    (void)L;
    return 0;
}
int LuaBridge::lua_get_method_info(lua_State* L) {
    (void)L;
    return 0;
}
int LuaBridge::lua_get_fields(lua_State* L) {
    (void)L;
    return 0;
}
int LuaBridge::lua_get_field_info(lua_State* L) {
    (void)L;
    return 0;
}
int LuaBridge::lua_get_node_type(lua_State* L) {
    (void)L;
    return 0;
}
int LuaBridge::lua_get_node_name(lua_State* L) {
    (void)L;
    return 0;
}
int LuaBridge::lua_get_node_location(lua_State* L) {
    (void)L;
    return 0;
}
int LuaBridge::lua_get_children(lua_State* L) {
    (void)L;
    return 0;
}
int LuaBridge::lua_get_parent(lua_State* L) {
    (void)L;
    return 0;
}
int LuaBridge::lua_get_source_range(lua_State* L) {
    (void)L;
    return 0;
}

int LuaBridge::lua_get_files(lua_State* L) {
    (void)L;
    return 0;
}

int LuaBridge::lua_get_functions(lua_State* L) {
    (void)L;
    return 0;
}

int LuaBridge::lua_get_enums(lua_State* L) {
    (void)L;
    return 0;
}

int LuaBridge::lua_get_namespaces(lua_State* L) {
    (void)L;
    return 0;
}

int LuaBridge::lua_get_switches(lua_State* L) {
    (void)L;
    return 0;
}

int LuaBridge::lua_get_usings(lua_State* L) {
    (void)L;
    return 0;
}

int LuaBridge::lua_get_includes(lua_State* L) {
    (void)L;
    return 0;
}

int LuaBridge::lua_get_file_info(lua_State* L) {
    (void)L;
    return 0;
}

int LuaBridge::lua_get_typedefs(lua_State* L) {
    (void)L;
    return 0;
}

int LuaBridge::lua_get_typedef_info(lua_State* L) {
    (void)L;
    return 0;
}

int LuaBridge::lua_get_variables(lua_State* L) {
    (void)L;
    return 0;
}

int LuaBridge::lua_get_variable_info(lua_State* L) {
    (void)L;
    return 0;
}

int LuaBridge::lua_get_macros(lua_State* L) {
    (void)L;
    return 0;
}

int LuaBridge::lua_get_macro_info(lua_State* L) {
    (void)L;
    return 0;
}

int LuaBridge::lua_get_if_statements(lua_State* L) {
    (void)L;
    return 0;
}

int LuaBridge::lua_get_loops(lua_State* L) {
    (void)L;
    return 0;
}

int LuaBridge::lua_get_try_statements(lua_State* L) {
    (void)L;
    return 0;
}

int LuaBridge::lua_get_comments(lua_State* L) {
    (void)L;
    return 0;
}

int LuaBridge::lua_get_constructors(lua_State* L) {
    (void)L;
    return 0;
}

int LuaBridge::lua_get_constructor_info(lua_State* L) {
    (void)L;
    return 0;
}

int LuaBridge::lua_get_destructors(lua_State* L) {
    (void)L;
    return 0;
}

int LuaBridge::lua_get_destructor_info(lua_State* L) {
    (void)L;
    return 0;
}

int LuaBridge::lua_get_operators(lua_State* L) {
    (void)L;
    return 0;
}

int LuaBridge::lua_get_operator_info(lua_State* L) {
    (void)L;
    return 0;
}

int LuaBridge::lua_get_templates(lua_State* L) {
    (void)L;
    return 0;
}

int LuaBridge::lua_get_template_info(lua_State* L) {
    (void)L;
    return 0;
}

int LuaBridge::lua_get_lambdas(lua_State* L) {
    (void)L;
    return 0;
}

int LuaBridge::lua_get_lambda_info(lua_State* L) {
    (void)L;
    return 0;
}

int LuaBridge::lua_get_friends(lua_State* L) {
    (void)L;
    return 0;
}

int LuaBridge::lua_get_static_asserts(lua_State* L) {
    (void)L;
    return 0;
}

int LuaBridge::lua_get_call_graph(lua_State* L) {
    (void)L;
    return 0;
}

int LuaBridge::lua_get_function_calls(lua_State* L) {
    (void)L;
    return 0;
}

int LuaBridge::lua_get_callers(lua_State* L) {
    (void)L;
    return 0;
}

int LuaBridge::lua_get_callees(lua_State* L) {
    (void)L;
    return 0;
}

int LuaBridge::lua_get_return_statements(lua_State* L) {
    (void)L;
    return 0;
}

int LuaBridge::lua_get_inheritance_tree(lua_State* L) {
    (void)L;
    return 0;
}

int LuaBridge::lua_get_attributes(lua_State* L) {
    (void)L;
    return 0;
}

// Enhanced AST APIs stubs
int LuaBridge::lua_get_classes_with_info(lua_State* L) {
    (void)L;
    return 0;
}

int LuaBridge::lua_get_methods_with_info(lua_State* L) {
    (void)L;
    return 0;
}

int LuaBridge::lua_get_all_methods(lua_State* L) {
    (void)L;
    return 0;
}

int LuaBridge::lua_get_function_info(lua_State* L) {
    (void)L;
    return 0;
}

int LuaBridge::lua_get_function_parameters(lua_State* L) {
    (void)L;
    return 0;
}

int LuaBridge::lua_get_classes_in_namespace(lua_State* L) {
    (void)L;
    return 0;
}

int LuaBridge::lua_get_functions_in_namespace(lua_State* L) {
    (void)L;
    return 0;
}

int LuaBridge::lua_get_include_details(lua_State* L) {
    (void)L;
    return 0;
}

int LuaBridge::lua_get_namespace_info(lua_State* L) {
    (void)L;
    return 0;
}

int LuaBridge::lua_get_class_methods_by_access(lua_State* L) {
    (void)L;
    return 0;
}

int LuaBridge::lua_get_class_fields_by_access(lua_State* L) {
    (void)L;
    return 0;
}

int LuaBridge::lua_is_call_allowed(lua_State* L) {
    (void)L;
    return 0;
}

int LuaBridge::lua_get_all_using_declarations(lua_State* L) {
    (void)L;
    return 0;
}

int LuaBridge::lua_get_base_classes(lua_State* L) {
    (void)L;
    return 0;
}

int LuaBridge::lua_print(lua_State* L) {
    (void)L;
    return 0;
}

#endif  // HAVE_LUAJIT

}  // namespace lua
}  // namespace cclint
