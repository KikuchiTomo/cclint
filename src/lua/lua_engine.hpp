#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// LuaJIT のヘッダー（条件付きコンパイル）
#ifdef HAVE_LUAJIT
extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}
#else
// LuaJIT がない場合はスタブ定義
struct lua_State;
#endif

namespace cclint {
namespace lua {

/// Luaエンジン
/// Lua VM の初期化と管理を行う
class LuaEngine {
public:
    /// コンストラクタ
    LuaEngine();

    /// デストラクタ
    ~LuaEngine();

    // コピー禁止
    LuaEngine(const LuaEngine&) = delete;
    LuaEngine& operator=(const LuaEngine&) = delete;

    /// Luaスクリプトをロード
    /// @param script_path スクリプトのパス
    /// @return 成功した場合true
    bool load_script(const std::string& script_path);

    /// Luaスクリプトを実行（文字列から）
    /// @param script スクリプトの内容
    /// @return 成功した場合true
    bool execute_string(const std::string& script);

    /// Lua関数を呼び出す
    /// @param function_name 関数名
    /// @param args 引数のリスト
    /// @return 成功した場合true
    bool call_function(const std::string& function_name, const std::vector<std::string>& args = {});

    /// エラーメッセージを取得
    std::string get_error_message() const { return error_message_; }

    /// Lua State を取得（低レベルアクセス用）
    lua_State* get_state() const { return L_; }

    /// LuaJIT が利用可能かどうか
    static bool is_available();

private:
    lua_State* L_;
    std::string error_message_;

    /// サンドボックスを設定
    void setup_sandbox();

    /// C++ API を登録
    void register_cpp_api();
};

}  // namespace lua
}  // namespace cclint
