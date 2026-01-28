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

#endif  // HAVE_LUAJIT

}  // namespace lua
}  // namespace cclint
