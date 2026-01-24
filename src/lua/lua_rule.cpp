#include "lua/lua_rule.hpp"
#include "utils/logger.hpp"

namespace cclint {
namespace lua {

LuaRule::LuaRule(const std::string& script_path,
                 const std::string& rule_name)
    : script_path_(script_path), rule_name_(rule_name),
      description_("Lua rule"), category_("custom") {

    lua_engine_ = std::make_shared<LuaEngine>();

    if (LuaEngine::is_available()) {
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
    // TODO: パラメータをLuaスクリプトに渡す
    (void)params;
}

void LuaRule::check_file(const std::string& file_path,
                         const std::string& content,
                         diagnostic::DiagnosticEngine& diag_engine) {
    if (!loaded_) {
        return;
    }

    // TODO: Luaスクリプトの check_file 関数を呼び出す
    // ここでは、診断エンジンへのコールバックを設定して、
    // Luaから report_error/report_warning を呼び出せるようにする
    (void)file_path;
    (void)content;
    (void)diag_engine;

    utils::Logger::instance().debug(
        "LuaRule::check_file called for: " + file_path +
        " (not yet implemented)");
}

void LuaRule::check_ast(clang::ASTUnit* ast_unit,
                        diagnostic::DiagnosticEngine& diag_engine) {
    if (!loaded_) {
        return;
    }

    // TODO: Luaスクリプトの check_ast 関数を呼び出す
    (void)ast_unit;
    (void)diag_engine;

    utils::Logger::instance().debug("LuaRule::check_ast called (not yet "
                                     "implemented)");
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
    // TODO: Luaスクリプトから description, category を取得
    // 例: rule.description, rule.category などのグローバル変数
}

} // namespace lua
} // namespace cclint
