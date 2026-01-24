#include "lua/lua_rule.hpp"
#include "utils/logger.hpp"

#include <sstream>

namespace cclint {
namespace lua {

LuaRule::LuaRule(const std::string& script_path,
                 const std::string& rule_name)
    : script_path_(script_path), rule_name_(rule_name),
      description_("Lua rule"), category_("custom") {

    lua_engine_ = std::make_shared<LuaEngine>();

    if (LuaEngine::is_available()) {
        // LuaBridgeを作成
        lua_bridge_ = std::make_shared<LuaBridge>(lua_engine_);
        lua_bridge_->register_api();

        load_script();
        if (loaded_) {
            load_metadata();
        }
    } else {
        load_error_ = "LuaJIT is not available";
        utils::Logger::instance().warning(
            "Cannot load Lua rule (LuaJIT not available): " + rule_name_);
    }
}

void LuaRule::initialize(const rules::RuleParameters& params) {
    if (!loaded_ || !LuaEngine::is_available()) {
        return;
    }

#ifdef HAVE_LUAJIT
    lua_State* L = lua_engine_->get_state();

    // パラメータをLuaのグローバルテーブルとして設定
    lua_newtable(L);
    for (const auto& [key, value] : params) {
        lua_pushstring(L, key.c_str());
        lua_pushstring(L, value.c_str());
        lua_settable(L, -3);
    }
    lua_setglobal(L, "rule_params");

    utils::Logger::instance().debug("Lua rule parameters set for: " +
                                     rule_name_);
#else
    (void)params;
#endif
}

void LuaRule::check_file(const std::string& file_path,
                         const std::string& content,
                         diagnostic::DiagnosticEngine& diag_engine) {
    if (!loaded_ || !LuaEngine::is_available()) {
        return;
    }

#ifdef HAVE_LUAJIT
    // 診断エンジンとファイルパスを設定
    lua_bridge_->set_diagnostic_engine(&diag_engine);
    lua_bridge_->set_current_file(file_path);

    // ファイル内容をLuaに渡す
    push_file_content_to_lua(file_path, content);

    // check_file 関数を呼び出す
    lua_State* L = lua_engine_->get_state();
    lua_getglobal(L, "check_file");

    if (lua_isfunction(L, -1)) {
        // file_path を引数として渡す
        lua_pushstring(L, file_path.c_str());

        if (lua_pcall(L, 1, 0, 0) != 0) {
            std::string error = lua_tostring(L, -1);
            lua_pop(L, 1);
            utils::Logger::instance().error("Lua rule execution failed: " +
                                             rule_name_ + " - " + error);
        }
    } else {
        lua_pop(L, 1);
        utils::Logger::instance().debug("Lua rule " + rule_name_ +
                                         " does not have check_file function");
    }
#else
    (void)file_path;
    (void)content;
    (void)diag_engine;
#endif
}

void LuaRule::check_ast(clang::ASTUnit* ast_unit,
                        diagnostic::DiagnosticEngine& diag_engine) {
    if (!loaded_) {
        return;
    }

    // AST解析はLLVM/Clangに依存するため、現時点では未実装
    (void)ast_unit;
    (void)diag_engine;
}

void LuaRule::check_ast(const std::string& file_path,
                        std::shared_ptr<parser::TranslationUnitNode> ast,
                        diagnostic::DiagnosticEngine& diag_engine) {
    if (!loaded_ || !LuaEngine::is_available()) {
        return;
    }

#ifdef HAVE_LUAJIT
    // 診断エンジン、ファイルパス、ASTを設定
    lua_bridge_->set_diagnostic_engine(&diag_engine);
    lua_bridge_->set_current_file(file_path);
    lua_bridge_->set_current_ast(ast);

    // check_ast 関数を呼び出す
    lua_State* L = lua_engine_->get_state();
    lua_getglobal(L, "check_ast");

    if (lua_isfunction(L, -1)) {
        // file_path を引数として渡す
        lua_pushstring(L, file_path.c_str());

        if (lua_pcall(L, 1, 0, 0) != 0) {
            std::string error = lua_tostring(L, -1);
            lua_pop(L, 1);
            utils::Logger::instance().error("Lua AST rule execution failed: " +
                                             rule_name_ + " - " + error);
        }
    } else {
        lua_pop(L, 1);
        utils::Logger::instance().debug("Lua rule " + rule_name_ +
                                         " does not have check_ast function");
    }
#else
    (void)file_path;
    (void)ast;
    (void)diag_engine;
#endif
}

void LuaRule::load_script() {
    if (!lua_engine_->load_script(script_path_)) {
        loaded_ = false;
        load_error_ = lua_engine_->get_error_message();
        utils::Logger::instance().error("Failed to load Lua rule: " +
                                         rule_name_ + " - " + load_error_);
        return;
    }

    loaded_ = true;
    utils::Logger::instance().info("Loaded Lua rule: " + rule_name_ +
                                    " from " + script_path_);
}

void LuaRule::load_metadata() {
#ifdef HAVE_LUAJIT
    lua_State* L = lua_engine_->get_state();

    // グローバル変数 rule_description を取得
    lua_getglobal(L, "rule_description");
    if (lua_isstring(L, -1)) {
        description_ = lua_tostring(L, -1);
    }
    lua_pop(L, 1);

    // グローバル変数 rule_category を取得
    lua_getglobal(L, "rule_category");
    if (lua_isstring(L, -1)) {
        category_ = lua_tostring(L, -1);
    }
    lua_pop(L, 1);

    utils::Logger::instance().debug("Loaded Lua rule metadata: " +
                                     rule_name_ + " [" + category_ + "]");
#endif
}

void LuaRule::push_file_content_to_lua(const std::string& file_path,
                                        const std::string& content) {
#ifdef HAVE_LUAJIT
    lua_State* L = lua_engine_->get_state();

    // グローバル変数 file_content を設定
    lua_pushstring(L, content.c_str());
    lua_setglobal(L, "file_content");

    // ファイルパスも設定
    lua_pushstring(L, file_path.c_str());
    lua_setglobal(L, "file_path");

    // ファイル内容を行ごとに分割してテーブルとして設定
    lua_newtable(L);
    std::istringstream stream(content);
    std::string line;
    int line_num = 1;

    while (std::getline(stream, line)) {
        lua_pushinteger(L, line_num);
        lua_pushstring(L, line.c_str());
        lua_settable(L, -3);
        line_num++;
    }

    lua_setglobal(L, "file_lines");
#else
    (void)file_path;
    (void)content;
#endif
}

} // namespace lua
} // namespace cclint
