# コード実装例とスニペット

**最終更新**: 2026-01-28

このドキュメントは実装時に参考にできるコードスニペットを集めたものです。

---

## 1. ASTノードの追加

### ast.hpp - ノードクラス定義

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

/// ループ文
class LoopStatementNode : public ASTNode {
public:
    enum class LoopType { For, While, DoWhile };
    LoopType loop_type;
    bool has_braces = false;

    LoopStatementNode(LoopType type)
        : ASTNode(ASTNodeType::LoopStatement), loop_type(type) {}
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

---

## 2. SimpleParserの拡張

### インデント情報の記録

```cpp
// simple_parser.hppに追加
class SimpleParser {
    // ...
private:
    struct IndentInfo {
        int count;
        bool uses_tabs;
    };
    std::map<int, IndentInfo> indent_info_;
    int last_line_ = -1;

    void record_indent_info(int line);
};

// simple_parser.cppに実装
void SimpleParser::record_indent_info(int line) {
    if (line >= file_lines_.size()) return;

    const std::string& line_str = file_lines_[line];
    int indent_count = 0;
    bool uses_tabs = false;

    for (char ch : line_str) {
        if (ch == ' ') {
            indent_count++;
        } else if (ch == '\t') {
            uses_tabs = true;
            indent_count += 4;  // タブは4スペース相当
        } else {
            break;
        }
    }

    indent_info_[line] = {indent_count, uses_tabs};
}
```

### if文のパース

```cpp
std::shared_ptr<ASTNode> SimpleParser::parse_if_statement() {
    auto node = std::make_shared<IfStatementNode>();
    node->position = current_position();

    // 'if' キーワード消費
    advance();  // 'if'

    // '(' 消費
    if (current_token_.type == TokenType::LeftParen) {
        advance();
    }

    // 条件式をスキップ（括弧のネストを考慮）
    int paren_depth = 1;
    while (paren_depth > 0 && !is_eof()) {
        if (current_token_.type == TokenType::LeftParen) {
            paren_depth++;
        } else if (current_token_.type == TokenType::RightParen) {
            paren_depth--;
        }
        advance();
    }

    // ブレースの有無をチェック
    node->has_braces = (current_token_.type == TokenType::LeftBrace);

    // 本体をパース
    if (node->has_braces) {
        auto body = parse_compound_statement();
        node->children.push_back(body);
    } else {
        auto stmt = parse_statement();
        node->children.push_back(stmt);
    }

    // else句のチェック
    if (current_token_.type == TokenType::Keyword &&
        current_token_.value == "else") {
        node->has_else = true;
        advance();  // 'else'

        // else本体をパース
        if (current_token_.type == TokenType::LeftBrace) {
            auto else_body = parse_compound_statement();
            node->children.push_back(else_body);
        } else {
            auto else_stmt = parse_statement();
            node->children.push_back(else_stmt);
        }
    }

    return node;
}
```

### マクロのパース

```cpp
std::shared_ptr<ASTNode> SimpleParser::parse_macro() {
    auto node = std::make_shared<MacroNode>();
    node->position = current_position();

    // '#define' を消費
    advance();  // '#'
    advance();  // 'define'

    // マクロ名
    if (current_token_.type == TokenType::Identifier) {
        node->name = current_token_.value;
        advance();
    }

    // マクロ関数のチェック
    if (current_token_.type == TokenType::LeftParen) {
        node->is_function = true;
        advance();  // '('

        // パラメータをパース
        while (current_token_.type != TokenType::RightParen && !is_eof()) {
            if (current_token_.type == TokenType::Identifier) {
                node->parameters.push_back(current_token_.value);
                advance();
            }

            if (current_token_.type == TokenType::Comma) {
                advance();
            }
        }

        if (current_token_.type == TokenType::RightParen) {
            advance();
        }
    }

    // 定義本体（行末まで）
    std::string definition;
    while (current_token_.line == node->position.line && !is_eof()) {
        definition += current_token_.value + " ";
        advance();
    }
    node->definition = trim(definition);

    return node;
}
```

---

## 3. Lua API実装

### get_files()の実装

```cpp
int LuaBridge::lua_get_files(lua_State* L) {
    // Bridgeインスタンスを取得
    lua_getglobal(L, "__cclint_bridge");
    LuaBridge* bridge = static_cast<LuaBridge*>(lua_touserdata(L, -1));
    lua_pop(L, 1);

    if (!bridge) {
        lua_newtable(L);
        return 1;
    }

    // ファイル配列を作成
    lua_newtable(L);

    std::string file_path = bridge->current_file_;

    lua_pushnumber(L, 1);
    lua_newtable(L);

    // path
    lua_pushstring(L, "path");
    lua_pushstring(L, file_path.c_str());
    lua_settable(L, -3);

    // name
    lua_pushstring(L, "name");
    std::string name = get_basename(file_path);
    lua_pushstring(L, name.c_str());
    lua_settable(L, -3);

    // extension
    lua_pushstring(L, "extension");
    std::string ext = get_extension(file_path);
    lua_pushstring(L, ext.c_str());
    lua_settable(L, -3);

    // is_header
    lua_pushstring(L, "is_header");
    bool is_header = (ext == "h" || ext == "hpp" || ext == "hxx");
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
```

### get_ifs()の実装

```cpp
// ヘルパー関数: ASTからIfStatementNodeを収集
void collect_if_statements(std::shared_ptr<ASTNode> node,
                          std::vector<std::shared_ptr<IfStatementNode>>& result) {
    if (!node) return;

    if (node->type == ASTNodeType::IfStatement) {
        result.push_back(std::static_pointer_cast<IfStatementNode>(node));
    }

    for (auto& child : node->children) {
        collect_if_statements(child, result);
    }
}

int LuaBridge::lua_get_ifs(lua_State* L) {
    lua_getglobal(L, "__cclint_bridge");
    LuaBridge* bridge = static_cast<LuaBridge*>(lua_touserdata(L, -1));
    lua_pop(L, 1);

    if (!bridge || !bridge->current_ast_) {
        lua_newtable(L);
        return 1;
    }

    // IfStatementNodeを収集
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

        lua_pushstring(L, "column");
        lua_pushnumber(L, if_stmt->position.column);
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

### メソッドチェーン実装

```cpp
// lua_bridge.cppに追加
void LuaBridge::setup_array_metatable(lua_State* L, int table_index) {
    // メタテーブルを作成
    lua_newtable(L);

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

    // メタテーブルを設定
    lua_setmetatable(L, table_index);
}

// eachの実装
int LuaBridge::lua_array_each(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    luaL_checktype(L, 2, LUA_TFUNCTION);

    lua_pushnil(L);
    while (lua_next(L, 1) != 0) {
        // キーをコピー
        lua_pushvalue(L, -2);

        // コールバック関数をコピー
        lua_pushvalue(L, 2);

        // 値をプッシュ
        lua_pushvalue(L, -3);

        // コールバック実行
        lua_call(L, 1, 0);

        // 値をポップ
        lua_pop(L, 2);
    }

    return 0;
}

// filterの実装
int LuaBridge::lua_array_filter(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    luaL_checktype(L, 2, LUA_TFUNCTION);

    // 結果テーブルを作成
    lua_newtable(L);
    int result_index = 1;

    lua_pushnil(L);
    while (lua_next(L, 1) != 0) {
        // 述語関数を呼び出し
        lua_pushvalue(L, 2);  // 述語関数
        lua_pushvalue(L, -2); // 値
        lua_call(L, 1, 1);

        // 結果がtrueなら結果テーブルに追加
        if (lua_toboolean(L, -1)) {
            lua_pop(L, 1);  // 結果をポップ

            lua_pushnumber(L, result_index++);
            lua_pushvalue(L, -2);  // 値
            lua_settable(L, -5);   // 結果テーブルに設定
        } else {
            lua_pop(L, 1);  // 結果をポップ
        }

        lua_pop(L, 1);  // 値をポップ
    }

    return 1;
}
```

---

## 4. コールグラフビルダー

### call_graph.hpp

```cpp
#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace cclint {
namespace parser {
class TranslationUnitNode;
}

namespace engine {

class CallGraph {
public:
    CallGraph() = default;

    void add_call(const std::string& caller, const std::string& callee);
    std::vector<std::string> get_callers(const std::string& function) const;
    std::vector<std::string> get_callees(const std::string& function) const;

private:
    std::unordered_map<std::string, std::vector<std::string>> callers_;
    std::unordered_map<std::string, std::vector<std::string>> callees_;
};

class CallGraphBuilder {
public:
    static std::shared_ptr<CallGraph> build(
        std::shared_ptr<parser::TranslationUnitNode> ast);

private:
    static void analyze_function(
        std::shared_ptr<parser::ASTNode> node,
        const std::string& current_function,
        CallGraph& graph);
};

}  // namespace engine
}  // namespace cclint
```

### call_graph.cpp

```cpp
#include "call_graph.hpp"
#include "parser/ast.hpp"

namespace cclint {
namespace engine {

void CallGraph::add_call(const std::string& caller, const std::string& callee) {
    callees_[caller].push_back(callee);
    callers_[callee].push_back(caller);
}

std::vector<std::string> CallGraph::get_callers(const std::string& function) const {
    auto it = callers_.find(function);
    if (it != callers_.end()) {
        return it->second;
    }
    return {};
}

std::vector<std::string> CallGraph::get_callees(const std::string& function) const {
    auto it = callees_.find(function);
    if (it != callees_.end()) {
        return it->second;
    }
    return {};
}

std::shared_ptr<CallGraph> CallGraphBuilder::build(
    std::shared_ptr<parser::TranslationUnitNode> ast) {

    auto graph = std::make_shared<CallGraph>();

    // ASTを走査して関数を見つける
    std::function<void(std::shared_ptr<parser::ASTNode>)> visit;
    visit = [&](std::shared_ptr<parser::ASTNode> node) {
        if (!node) return;

        if (node->type == parser::ASTNodeType::Function) {
            // 関数内の呼び出しを解析
            analyze_function(node, node->name, *graph);
        }

        for (auto& child : node->children) {
            visit(child);
        }
    };

    visit(ast);
    return graph;
}

void CallGraphBuilder::analyze_function(
    std::shared_ptr<parser::ASTNode> node,
    const std::string& current_function,
    CallGraph& graph) {

    if (!node) return;

    // CallExpressionNodeを探す
    if (node->type == parser::ASTNodeType::CallExpression) {
        auto call = std::static_pointer_cast<parser::CallExpressionNode>(node);
        graph.add_call(current_function, call->function_name);
    }

    for (auto& child : node->children) {
        analyze_function(child, current_function, graph);
    }
}

}  // namespace engine
}  // namespace cclint
```

---

## 5. 組み込みルールのLua版

### max_line_length.lua

```lua
rule_description = "Check maximum line length"
rule_category = "style"

function check_file()
    local max_length = 80
    if rule_params and rule_params.max_length then
        max_length = tonumber(rule_params.max_length) or 80
    end

    for line_num, line in ipairs(file_lines) do
        if #line > max_length then
            cclint.report_warning(
                line_num,
                max_length + 1,
                string.format("Line exceeds maximum length of %d (found %d)",
                              max_length, #line)
            )
        end
    end
end
```

### if_braces.lua（新規）

```lua
rule_description = "Enforce braces for all if statements"
rule_category = "style"

function check_ast()
    local ifs = cclint.get_ifs()

    if not ifs then
        return
    end

    for _, if_stmt in ipairs(ifs) do
        if not if_stmt.has_braces then
            cclint.report_warning(
                if_stmt.line,
                if_stmt.column,
                "if statement must have braces, even for single-line body"
            )
        end
    end
end
```

---

## 6. ユーティリティ関数

### ファイル名処理

```cpp
std::string get_basename(const std::string& path) {
    size_t pos = path.find_last_of("/\\");
    if (pos == std::string::npos) {
        return path;
    }
    return path.substr(pos + 1);
}

std::string get_extension(const std::string& path) {
    size_t pos = path.find_last_of('.');
    if (pos == std::string::npos) {
        return "";
    }
    return path.substr(pos + 1);
}
```

### 文字列トリム

```cpp
std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) {
        return "";
    }

    size_t end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}
```

---

**これらのコード例を参考に実装を進めてください。コピペして適宜修正して使用できます。**
