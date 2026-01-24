#include "lua/lua_engine.hpp"

#include <fstream>
#include <sstream>

#include "utils/logger.hpp"

namespace cclint {
namespace lua {

#ifdef HAVE_LUAJIT

LuaEngine::LuaEngine() : L_(nullptr) {
    L_ = luaL_newstate();
    if (!L_) {
        error_message_ = "Failed to create Lua state";
        return;
    }

    // 標準ライブラリをロード
    luaL_openlibs(L_);

    // サンドボックス設定
    setup_sandbox();

    // C++ API登録
    register_cpp_api();

    utils::Logger::instance().debug("Lua engine initialized");
}

LuaEngine::~LuaEngine() {
    if (L_) {
        lua_close(L_);
        L_ = nullptr;
    }
}

bool LuaEngine::load_script(const std::string& script_path) {
    if (!L_) {
        error_message_ = "Lua state not initialized";
        return false;
    }

    // ファイルを読み込む
    std::ifstream file(script_path);
    if (!file.is_open()) {
        error_message_ = "Failed to open script: " + script_path;
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string script_content = buffer.str();

    return execute_string(script_content);
}

bool LuaEngine::execute_string(const std::string& script) {
    if (!L_) {
        error_message_ = "Lua state not initialized";
        return false;
    }

    if (luaL_loadstring(L_, script.c_str()) != 0) {
        error_message_ = lua_tostring(L_, -1);
        lua_pop(L_, 1);
        return false;
    }

    if (lua_pcall(L_, 0, 0, 0) != 0) {
        error_message_ = lua_tostring(L_, -1);
        lua_pop(L_, 1);
        return false;
    }

    return true;
}

bool LuaEngine::call_function(const std::string& function_name,
                              const std::vector<std::string>& args) {
    if (!L_) {
        error_message_ = "Lua state not initialized";
        return false;
    }

    lua_getglobal(L_, function_name.c_str());
    if (!lua_isfunction(L_, -1)) {
        error_message_ = "Function not found: " + function_name;
        lua_pop(L_, 1);
        return false;
    }

    // 引数をスタックにプッシュ
    for (const auto& arg : args) {
        lua_pushstring(L_, arg.c_str());
    }

    // 関数を呼び出す
    if (lua_pcall(L_, args.size(), 0, 0) != 0) {
        error_message_ = lua_tostring(L_, -1);
        lua_pop(L_, 1);
        return false;
    }

    return true;
}

void LuaEngine::setup_sandbox() {
    // セキュリティのため、危険な関数を無効化
    execute_string(R"(
        -- ファイルI/O制限
        io = nil
        -- システムコマンド実行禁止
        os.execute = nil
        os.exit = nil
        os.remove = nil
        os.rename = nil
        -- モジュールロード制限
        dofile = nil
        loadfile = nil
        require = nil
    )");

    // リソース制限の設定
    // メモリ制限: 100MB（LuaJITのデフォルトは十分大きい）
    // スタック深度制限: デフォルトで設定済み（lua_checkstack）

    // デバッグフック設定（実行時間制限用）
    // 注: この実装は簡易的。より厳密には instruction count で制限すべき
    lua_sethook(L_, nullptr, 0, 0);  // デフォルトではフックなし
}

void LuaEngine::register_cpp_api() {
    // TODO: C++ APIの登録（Milestone 3で実装）
    // 診断報告API、ASTアクセスAPI、ユーティリティAPI等
}

bool LuaEngine::is_available() {
    return true;
}

#else  // HAVE_LUAJIT が定義されていない場合（スタブ実装）

LuaEngine::LuaEngine() : L_(nullptr) {
    error_message_ = "LuaJIT is not available (not compiled with LuaJIT support)";
    utils::Logger::instance().warning("LuaJIT is not available");
}

LuaEngine::~LuaEngine() {}

bool LuaEngine::load_script(const std::string& script_path) {
    (void)script_path;
    return false;
}

bool LuaEngine::execute_string(const std::string& script) {
    (void)script;
    return false;
}

bool LuaEngine::call_function(const std::string& function_name,
                              const std::vector<std::string>& args) {
    (void)function_name;
    (void)args;
    return false;
}

void LuaEngine::setup_sandbox() {}

void LuaEngine::register_cpp_api() {}

bool LuaEngine::is_available() {
    return false;
}

#endif  // HAVE_LUAJIT

}  // namespace lua
}  // namespace cclint
