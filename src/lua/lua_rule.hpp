#pragma once

#include "rules/rule_base.hpp"
#include "lua/lua_engine.hpp"
#include "lua/lua_bridge.hpp"

#include <memory>
#include <string>

namespace cclint {
namespace lua {

/// Luaスクリプトをルールとして扱うラッパークラス
class LuaRule : public rules::RuleBase {
public:
    /// コンストラクタ
    /// @param script_path Luaスクリプトのパス
    /// @param rule_name ルール名
    explicit LuaRule(const std::string& script_path,
                     const std::string& rule_name);

    std::string name() const override { return rule_name_; }
    std::string description() const override { return description_; }
    std::string category() const override { return category_; }

    void initialize(const rules::RuleParameters& params) override;

    void check_file(const std::string& file_path,
                    const std::string& content,
                    diagnostic::DiagnosticEngine& diag_engine) override;

    void check_ast(clang::ASTUnit* ast_unit,
                   diagnostic::DiagnosticEngine& diag_engine) override;

    void check_ast(const std::string& file_path,
                   std::shared_ptr<parser::TranslationUnitNode> ast,
                   diagnostic::DiagnosticEngine& diag_engine) override;

    /// スクリプトのロードに成功したかどうか
    bool is_loaded() const { return loaded_; }

    /// ロードエラーメッセージを取得
    std::string get_load_error() const { return load_error_; }

private:
    std::string script_path_;
    std::string rule_name_;
    std::string description_;
    std::string category_;
    std::shared_ptr<LuaEngine> lua_engine_;
    std::shared_ptr<LuaBridge> lua_bridge_;
    bool loaded_ = false;
    std::string load_error_;

    /// スクリプトをロード
    void load_script();

    /// Lua関数を呼び出してメタデータを取得
    void load_metadata();

    /// Luaにファイル内容を渡す
    void push_file_content_to_lua(const std::string& file_path,
                                   const std::string& content);
};

} // namespace lua
} // namespace cclint
