# 実装ガイド - 優先順位と具体的な手順

**最終更新**: 2026-01-28

## 実装の優先順位

### 最優先（MVP - 2週間で完成）

#### Week 1: ASTパーサー拡張とLua API基礎

**Day 1-2: ASTノード拡張**
- [x] SourcePositionにインデント情報追加（開始済み）
- [ ] ASTNodeTypeに新しいノードタイプ追加（開始済み）
- [ ] 新しいノードクラス定義（IfStatementNode等）
- [ ] ast.cppの実装

**Day 3-4: SimpleParser拡張**
- [ ] 制御構造のパース（if, switch, for, while）
- [ ] マクロ定義のパース
- [ ] インデント情報の記録
- [ ] ブレース有無の検出

**Day 5-7: 基本Lua API実装**
- [ ] lua_get_files()
- [ ] lua_get_functions()
- [ ] lua_get_ifs()
- [ ] lua_get_enums()
- [ ] lua_get_macros()

#### Week 2: 組み込みルール削除とドキュメント

**Day 8-9: コールグラフ構築**
- [ ] CallGraphBuilder実装
- [ ] 関数呼び出しの検出
- [ ] callers/callees記録
- [ ] lua_get_callers/callees実装

**Day 10: 組み込みルール削除**
- [ ] 組み込みルールのLua版作成
- [ ] C++実装削除
- [ ] ビルド確認

**Day 11-12: メソッドチェーン実装**
- [ ] Luaテーブルのメタテーブル設定
- [ ] .each(), .filter(), .map()実装
- [ ] 基本動作確認

**Day 13-14: ドキュメントと動作確認**
- [ ] README.md更新
- [ ] lua_api.md更新
- [ ] サンプルルール作成
- [ ] 統合テスト

---

## 詳細実装手順

### Phase 2.1: ASTノード追加

#### ファイル: src/parser/ast.hpp

1. 新しいノードクラスを追加:

```cpp
/// If文
class IfStatementNode : public ASTNode {
public:
    bool has_braces = false;
    bool has_else = false;

    IfStatementNode() : ASTNode(ASTNodeType::IfStatement) {}
};

/// Switch文
class SwitchStatementNode : public ASTNode {
public:
    bool has_default = false;
    int case_count = 0;

    SwitchStatementNode() : ASTNode(ASTNodeType::SwitchStatement) {}
};

/// ループ文（for/while/do-while）
class LoopStatementNode : public ASTNode {
public:
    enum class LoopType { For, While, DoWhile };
    LoopType loop_type;
    bool has_braces = false;

    LoopStatementNode() : ASTNode(ASTNodeType::LoopStatement) {}
};

/// Try-Catch文
class TryStatementNode : public ASTNode {
public:
    int catch_count = 0;
    bool has_finally = false;

    TryStatementNode() : ASTNode(ASTNodeType::TryStatement) {}
};

/// マクロ定義
class MacroNode : public ASTNode {
public:
    bool is_function = false;
    std::vector<std::string> parameters;
    std::string definition;

    MacroNode() : ASTNode(ASTNodeType::Macro) {}
};

/// 関数呼び出し
class CallExpressionNode : public ASTNode {
public:
    std::string function_name;
    std::vector<std::string> arguments;

    CallExpressionNode() : ASTNode(ASTNodeType::CallExpression) {}
};
```

#### ファイル: src/parser/ast.cpp

2. get_type_name()に新しいケースを追加:

```cpp
std::string ASTNode::get_type_name() const {
    switch (type) {
        // ... 既存のケース ...
        case ASTNodeType::IfStatement: return "IfStatement";
        case ASTNodeType::SwitchStatement: return "SwitchStatement";
        case ASTNodeType::LoopStatement: return "LoopStatement";
        case ASTNodeType::TryStatement: return "TryStatement";
        case ASTNodeType::Macro: return "Macro";
        case ASTNodeType::CallExpression: return "CallExpression";
        default: return "Unknown";
    }
}
```

### Phase 2.2: SimpleParserの拡張

#### ファイル: src/parser/simple_parser.cpp

1. インデント情報の記録:

```cpp
// トークンを読む際にインデント情報を保存
Token SimpleParser::consume_token() {
    if (current_token_.line != last_line_) {
        // 新しい行の場合、インデント情報を記録
        record_indent_info(current_token_);
        last_line_ = current_token_.line;
    }
    // ...
}

void SimpleParser::record_indent_info(const Token& token) {
    int indent_count = 0;
    bool uses_tabs = false;

    const std::string& line = get_line(token.line);
    for (char ch : line) {
        if (ch == ' ') {
            indent_count++;
        } else if (ch == '\t') {
            uses_tabs = true;
            indent_count += 4; // タブは4スペース相当
        } else {
            break;
        }
    }

    indent_info_[token.line] = {indent_count, uses_tabs};
}
```

2. 制御構造のパース:

```cpp
std::shared_ptr<ASTNode> SimpleParser::parse_if_statement() {
    auto node = std::make_shared<IfStatementNode>();
    node->position = current_position();

    consume_keyword("if");
    consume_token(); // '('
    // 条件式をスキップ
    skip_until(')');
    consume_token(); // ')'

    // ブレースの有無をチェック
    node->has_braces = (peek().type == TokenType::LeftBrace);

    // 本体をパース
    if (node->has_braces) {
        parse_compound_statement();
    } else {
        parse_statement();
    }

    // elseチェック
    if (peek_keyword("else")) {
        node->has_else = true;
        consume_keyword("else");
        // else本体をパース
    }

    return node;
}
```

3. マクロのパース:

```cpp
std::shared_ptr<ASTNode> SimpleParser::parse_macro() {
    auto node = std::make_shared<MacroNode>();
    node->position = current_position();

    consume_token(); // '#'
    consume_keyword("define");

    node->name = consume_identifier();

    // マクロ関数かチェック
    if (peek().type == TokenType::LeftParen) {
        node->is_function = true;
        consume_token(); // '('

        // パラメータをパース
        while (peek().type != TokenType::RightParen) {
            node->parameters.push_back(consume_identifier());
            if (peek().type == TokenType::Comma) {
                consume_token();
            }
        }
        consume_token(); // ')'
    }

    // 定義本体を取得（行末まで）
    node->definition = read_until_newline();

    return node;
}
```

### Phase 3: Lua API実装

#### ファイル: src/lua/lua_bridge.hpp

1. 新しいAPIの宣言を追加:

```cpp
class LuaBridge {
    // ... 既存のメンバ ...

private:
    // 新しいAPI
    static int lua_get_files(lua_State* L);
    static int lua_get_functions(lua_State* L);
    static int lua_get_mains(lua_State* L);
    static int lua_get_enums(lua_State* L);
    static int lua_get_ifs(lua_State* L);
    static int lua_get_switches(lua_State* L);
    static int lua_get_loops(lua_State* L);
    static int lua_get_macros(lua_State* L);
    static int lua_get_callers(lua_State* L);
    static int lua_get_callees(lua_State* L);
    static int lua_get_line_info(lua_State* L);
};
```

#### ファイル: src/lua/lua_bridge.cpp

2. register_api()に登録を追加:

```cpp
void LuaBridge::register_api() {
    lua_State* L = lua_engine_->get_state();

    // 既存のAPI登録...

    // 新しいAPI登録
    lua_register(L, "get_files", lua_get_files);
    lua_register(L, "get_functions", lua_get_functions);
    lua_register(L, "get_mains", lua_get_mains);
    lua_register(L, "get_enums", lua_get_enums);
    lua_register(L, "get_ifs", lua_get_ifs);
    // ...
}
```

3. APIの実装例（get_files）:

```cpp
int LuaBridge::lua_get_files(lua_State* L) {
    // 現在のASTを取得
    lua_getglobal(L, "__cclint_bridge");
    LuaBridge* bridge = static_cast<LuaBridge*>(lua_touserdata(L, -1));
    lua_pop(L, 1);

    if (!bridge || !bridge->current_ast_) {
        lua_newtable(L);
        return 1;
    }

    // ファイル一覧を作成
    lua_newtable(L);
    int index = 1;

    // TranslationUnitからファイル情報を取得
    std::string file_path = bridge->current_file_;
    lua_pushnumber(L, index);
    lua_newtable(L);

    // ファイル情報をテーブルに設定
    lua_pushstring(L, "path");
    lua_pushstring(L, file_path.c_str());
    lua_settable(L, -3);

    lua_pushstring(L, "name");
    std::string name = get_filename(file_path);
    lua_pushstring(L, name.c_str());
    lua_settable(L, -3);

    lua_settable(L, -3);

    return 1;
}
```

4. get_ifs()の実装:

```cpp
int LuaBridge::lua_get_ifs(lua_State* L) {
    lua_getglobal(L, "__cclint_bridge");
    LuaBridge* bridge = static_cast<LuaBridge*>(lua_touserdata(L, -1));
    lua_pop(L, 1);

    if (!bridge || !bridge->current_ast_) {
        lua_newtable(L);
        return 1;
    }

    // ASTを走査してIfStatementNodeを収集
    std::vector<std::shared_ptr<IfStatementNode>> if_stmts;
    collect_if_statements(bridge->current_ast_, if_stmts);

    // Luaテーブルを作成
    lua_newtable(L);
    for (size_t i = 0; i < if_stmts.size(); i++) {
        lua_pushnumber(L, i + 1);
        lua_newtable(L);

        auto if_stmt = if_stmts[i];

        lua_pushstring(L, "line");
        lua_pushnumber(L, if_stmt->position.line);
        lua_settable(L, -3);

        lua_pushstring(L, "has_braces");
        lua_pushboolean(L, if_stmt->has_braces);
        lua_settable(L, -3);

        lua_pushstring(L, "has_else");
        lua_pushboolean(L, if_stmt->has_else);
        lua_settable(L, -3);

        lua_settable(L, -3);
    }

    return 1;
}
```

### Phase 4: 組み込みルール削除

1. Luaルール作成（例: max_line_length.lua）:

```lua
-- scripts/rules/style/max_line_length.lua
rule_description = "Check maximum line length"
rule_category = "style"

function check_file()
    local max_length = 80
    if rule_params and rule_params.max_length then
        max_length = tonumber(rule_params.max_length)
    end

    for line_num, line in ipairs(file_lines) do
        if #line > max_length then
            cclint.report_warning(
                line_num, max_length + 1,
                string.format("Line exceeds maximum length of %d (found %d)",
                              max_length, #line)
            )
        end
    end
end
```

2. C++実装の削除:

```bash
git rm src/rules/builtin/max_line_length.cpp
git rm src/rules/builtin/max_line_length.hpp
git rm src/rules/builtin/header_guard.cpp
git rm src/rules/builtin/header_guard.hpp
git rm src/rules/builtin/function_complexity.cpp
git rm src/rules/builtin/function_complexity.hpp
git rm src/rules/builtin/naming_convention.cpp
git rm src/rules/builtin/naming_convention.hpp
```

3. CMakeLists.txtの更新:

```cmake
# src/CMakeLists.txt から builtin ルールのソースファイルを削除
# set(BUILTIN_RULES_SOURCES ...) の行を削除
```

### Phase 5: メソッドチェーン実装

#### ファイル: src/lua/lua_bridge.cpp

```cpp
// メタテーブルを設定してメソッドチェーンを可能にする
void LuaBridge::setup_method_chaining(lua_State* L) {
    // メタテーブルを作成
    luaL_newmetatable(L, "cclint_array");

    // __index メタメソッド
    lua_pushstring(L, "__index");
    lua_newtable(L);

    // each メソッド
    lua_pushstring(L, "each");
    lua_pushcfunction(L, lua_array_each);
    lua_settable(L, -3);

    // filter メソッド
    lua_pushstring(L, "filter");
    lua_pushcfunction(L, lua_array_filter);
    lua_settable(L, -3);

    // map メソッド
    lua_pushstring(L, "map");
    lua_pushcfunction(L, lua_array_map);
    lua_settable(L, -3);

    // count メソッド
    lua_pushstring(L, "count");
    lua_pushcfunction(L, lua_array_count);
    lua_settable(L, -3);

    lua_settable(L, -3);
    lua_pop(L, 1);
}

// each実装
int LuaBridge::lua_array_each(lua_State* L) {
    // 配列とコールバック関数を取得
    luaL_checktype(L, 1, LUA_TTABLE);
    luaL_checktype(L, 2, LUA_TFUNCTION);

    // 各要素に対してコールバックを実行
    lua_pushnil(L);
    while (lua_next(L, 1) != 0) {
        lua_pushvalue(L, 2); // コールバック関数
        lua_pushvalue(L, -2); // 値
        lua_call(L, 1, 0);
        lua_pop(L, 1);
    }

    return 0;
}
```

---

## 実装時の注意点

### ビルドとテスト

各機能実装後は必ずビルドとテストを行う:

```bash
# ビルド
cd build
cmake ..
make -j$(nproc)

# テスト
./cclint --config=../examples/configs/test.yaml g++ ../tests/samples/test.cpp
```

### デバッグ

Lua APIのデバッグ:

```bash
# verboseモードで実行
./cclint -vv --config=test.yaml g++ test.cpp
```

### コミット

小さい単位で頻繁にコミット:

```bash
git add src/parser/ast.hpp
git commit -m "feat(ast): Add IfStatementNode and related nodes"

git add src/parser/simple_parser.cpp
git commit -m "feat(parser): Implement if statement parsing"
```

---

## トラブルシューティング

### ビルドエラー

```bash
# クリーンビルド
rm -rf build
mkdir build && cd build
cmake ..
make
```

### Luaエラー

```bash
# Luaスクリプトの構文チェック
luac -p scripts/rules/test.lua
```

---

**実装を始める前に、必ず05_API_SPECIFICATION.mdと06_CODE_EXAMPLES.mdを確認してください。**
