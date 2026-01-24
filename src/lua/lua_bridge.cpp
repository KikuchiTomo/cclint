#include "lua/lua_bridge.hpp"
#include "utils/logger.hpp"
#include "utils/file_utils.hpp"

#include <regex>

namespace cclint {
namespace lua {

#ifdef HAVE_LUAJIT

// グローバルなブリッジインスタンス（Luaのコールバック用）
static LuaBridge* g_bridge = nullptr;

LuaBridge::LuaBridge(std::shared_ptr<LuaEngine> lua_engine)
    : lua_engine_(lua_engine) {
    g_bridge = this;
}

void LuaBridge::set_diagnostic_engine(diagnostic::DiagnosticEngine* diag_engine) {
    diag_engine_ = diag_engine;
}

void LuaBridge::set_current_file(const std::string& file_path) {
    current_file_ = file_path;
}

void LuaBridge::register_api() {
    if (!lua_engine_ || !LuaEngine::is_available()) {
        return;
    }

    lua_State* L = lua_engine_->get_state();

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

    // グローバルに設定
    lua_setglobal(L, "cclint");

    utils::Logger::instance().debug("Lua C++ API registered");
}

bool LuaBridge::call_function(const std::string& function_name,
                               const std::vector<std::string>& args) {
    return lua_engine_->call_function(function_name, args);
}

void LuaBridge::report_diagnostic(const std::string& file_path, int line,
                                   int column, const std::string& message,
                                   diagnostic::Severity severity) {
    if (!diag_engine_) {
        utils::Logger::instance().warning(
            "Diagnostic engine not set in LuaBridge");
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

#else // HAVE_LUAJIT が定義されていない場合（スタブ実装）

LuaBridge::LuaBridge(std::shared_ptr<LuaEngine> lua_engine)
    : lua_engine_(lua_engine) {}

void LuaBridge::set_diagnostic_engine(diagnostic::DiagnosticEngine* diag_engine) {
    (void)diag_engine;
}

void LuaBridge::set_current_file(const std::string& file_path) {
    (void)file_path;
}

void LuaBridge::register_api() {}

bool LuaBridge::call_function(const std::string& function_name,
                               const std::vector<std::string>& args) {
    (void)function_name;
    (void)args;
    return false;
}

void LuaBridge::report_diagnostic(const std::string& file_path, int line,
                                   int column, const std::string& message,
                                   diagnostic::Severity severity) {
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

#endif // HAVE_LUAJIT

} // namespace lua
} // namespace cclint
