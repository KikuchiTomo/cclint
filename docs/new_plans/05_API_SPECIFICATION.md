# Lua API 仕様書

**最終更新**: 2026-01-28

このドキュメントは実装する全てのLua APIの仕様を定義します。

---

## 1. ファイルレベルAPI

### `cclint.get_files()`

全ファイルの情報を取得します。

**パラメータ**: なし

**戻り値**: ファイル情報のテーブル配列

```lua
{
    {
        path = "/path/to/file.cpp",
        name = "file.cpp",
        extension = "cpp",
        is_header = false,
        is_implementation = true
    },
    ...
}
```

**実装場所**: `src/lua/lua_bridge.cpp::lua_get_files`

**C++実装**:
- TranslationUnitから現在のファイルパスを取得
- ファイル情報をLuaテーブルとして返す

---

## 2. 構造レベルAPI

### `cclint.get_functions()`

全ての関数（グローバル関数+メソッド）を取得します。

**パラメータ**: なし

**戻り値**: 関数情報のテーブル配列

```lua
{
    {
        name = "myFunction",
        return_type = "int",
        line = 42,
        file = "/path/to/file.cpp",
        is_method = false,
        class_name = nil,  -- メソッドの場合はクラス名
        access = "none",   -- "public", "protected", "private", "none"
        is_static = false,
        is_virtual = false,
        is_const = false
    },
    ...
}
```

**実装場所**: `src/lua/lua_bridge.cpp::lua_get_functions`

**C++実装**:
- ASTを再帰的に走査してFunctionNodeを収集
- 各関数の情報をLuaテーブルとして返す

---

### `cclint.get_mains()`

main関数のみを取得します。

**パラメータ**: なし

**戻り値**: main関数情報のテーブル配列（通常は1要素）

```lua
{
    {
        name = "main",
        return_type = "int",
        line = 10,
        file = "/path/to/main.cpp"
    }
}
```

**実装場所**: `src/lua/lua_bridge.cpp::lua_get_mains`

---

### `cclint.get_enums()`

全てのenum/enum classを取得します。

**パラメータ**: なし

**戻り値**: enum情報のテーブル配列

```lua
{
    {
        name = "Color",
        is_class = true,  -- enum class かどうか
        line = 15,
        values = {"Red", "Green", "Blue"},
        underlying_type = "int"
    },
    ...
}
```

**実装場所**: `src/lua/lua_bridge.cpp::lua_get_enums`

---

## 3. 制御構造API

### `cclint.get_ifs()`

全てのif文を取得します。

**パラメータ**: なし

**戻り値**: if文情報のテーブル配列

```lua
{
    {
        line = 20,
        column = 5,
        has_braces = true,
        has_else = false
    },
    ...
}
```

**実装場所**: `src/lua/lua_bridge.cpp::lua_get_ifs`

**C++実装**:
- ASTを走査してIfStatementNodeを収集

---

### `cclint.get_switches()`

全てのswitch文を取得します。

**パラメータ**: なし

**戻り値**: switch文情報のテーブル配列

```lua
{
    {
        line = 30,
        has_default = true,
        case_count = 5
    },
    ...
}
```

**実装場所**: `src/lua/lua_bridge.cpp::lua_get_switches`

---

### `cclint.get_loops()`

全てのループ文を取得します。

**パラメータ**: なし

**戻り値**: ループ情報のテーブル配列

```lua
{
    {
        line = 40,
        type = "for",  -- "for", "while", "do-while"
        has_braces = true
    },
    ...
}
```

**実装場所**: `src/lua/lua_bridge.cpp::lua_get_loops`

---

## 4. マクロAPI

### `cclint.get_macros()`

全てのマクロ定義を取得します。

**パラメータ**: なし

**戻り値**: マクロ情報のテーブル配列

```lua
{
    {
        name = "MAX_SIZE",
        line = 5,
        is_function = false,
        parameters = {},
        definition = "1024"
    },
    {
        name = "SQUARE",
        line = 10,
        is_function = true,
        parameters = {"x"},
        definition = "(x) * (x)"
    },
    ...
}
```

**実装場所**: `src/lua/lua_bridge.cpp::lua_get_macros`

---

## 5. 関係性API

### `function.callers()`

この関数を呼び出している関数の一覧を取得します。

**使用方法**:
```lua
local funcs = cclint.get_functions()
for _, func in ipairs(funcs) do
    local callers = func.callers()
    -- callersは関数情報のテーブル配列
end
```

**実装**:
- 関数情報テーブルにメタテーブルを設定
- `callers`メソッドを追加

**実装場所**: `src/lua/lua_bridge.cpp::lua_get_callers`

---

### `function.callees()`

この関数が呼び出している関数の一覧を取得します。

**使用方法**:
```lua
local funcs = cclint.get_functions()
for _, func in ipairs(funcs) do
    local callees = func.callees()
    -- calleesは関数情報のテーブル配列
end
```

**実装場所**: `src/lua/lua_bridge.cpp::lua_get_callees`

---

## 6. インデント・スタイルAPI

### `cclint.get_line_info(file_path, line_number)`

指定した行の詳細情報を取得します。

**パラメータ**:
- `file_path` (string): ファイルパス
- `line_number` (number): 行番号

**戻り値**: 行情報のテーブル

```lua
{
    line = 42,
    indent_count = 8,        -- インデント文字数
    indent_type = "space",   -- "space" or "tab"
    indent_level = 2,        -- インデントレベル（indent_count / 4）
    content = "    int x = 5;"
}
```

**実装場所**: `src/lua/lua_bridge.cpp::lua_get_line_info`

---

## 7. メソッドチェーンAPI

全ての配列型の戻り値に対して使用可能です。

### `array:each(callback)`

配列の各要素に対してコールバックを実行します。

**パラメータ**:
- `callback` (function): `function(element) ... end`

**戻り値**: なし

**使用例**:
```lua
cclint.get_files():each(function(file)
    print(file.path)
end)
```

**実装場所**: `src/lua/lua_bridge.cpp::lua_array_each`

---

### `array:filter(predicate)`

条件に一致する要素のみを抽出します。

**パラメータ**:
- `predicate` (function): `function(element) return bool end`

**戻り値**: フィルタリングされた配列

**使用例**:
```lua
local public_methods = cclint.get_functions():filter(function(func)
    return func.access == "public"
end)
```

**実装場所**: `src/lua/lua_bridge.cpp::lua_array_filter`

---

### `array:map(transform)`

各要素を変換した新しい配列を返します。

**パラメータ**:
- `transform` (function): `function(element) return new_value end`

**戻り値**: 変換された配列

**使用例**:
```lua
local function_names = cclint.get_functions():map(function(func)
    return func.name
end)
```

**実装場所**: `src/lua/lua_bridge.cpp::lua_array_map`

---

### `array:find(predicate)`

条件に一致する最初の要素を返します。

**パラメータ**:
- `predicate` (function): `function(element) return bool end`

**戻り値**: 見つかった要素、または nil

**使用例**:
```lua
local main_func = cclint.get_functions():find(function(func)
    return func.name == "main"
end)
```

**実装場所**: `src/lua/lua_bridge.cpp::lua_array_find`

---

### `array:count()`

配列の要素数を返します。

**パラメータ**: なし

**戻り値**: 要素数 (number)

**使用例**:
```lua
local method_count = cclint.get_methods("MyClass"):count()
```

**実装場所**: `src/lua/lua_bridge.cpp::lua_array_count`

---

## 8. ファイル区分API

### `files:header()`

ヘッダーファイルのみにフィルタリングします。

**戻り値**: ヘッダーファイルの配列

**使用例**:
```lua
cclint.get_files():header():each(function(file)
    print("Header: " .. file.path)
end)
```

**実装**: `filter`を使った内部実装
```lua
function array:header()
    return self:filter(function(file)
        return file.is_header
    end)
end
```

---

### `files:implementation()`

実装ファイルのみにフィルタリングします。

**戻り値**: 実装ファイルの配列

**使用例**:
```lua
cclint.get_files():implementation():each(function(file)
    print("Implementation: " .. file.path)
end)
```

---

## 9. ナビゲーション機能

### `file:classes()`

ファイル内のクラス一覧を取得します。

**戻り値**: クラス名の配列

**使用例**:
```lua
local files = cclint.get_files()
files:each(function(file)
    local classes = file:classes()
    -- ...
end)
```

**実装**: Luaテーブルのメタテーブルで実装

---

### `file:functions()`

ファイル内の関数一覧を取得します。

**戻り値**: 関数情報の配列

---

### `class:methods()`

クラスのメソッド一覧を取得します。（既存APIの改善版）

**戻り値**: メソッド情報の配列

---

## 実装の優先順位

### 最優先（Week 1）
1. `get_files()` - ファイル取得
2. `get_functions()` - 関数取得
3. `get_ifs()` - if文取得
4. `get_enums()` - enum取得
5. `get_macros()` - マクロ取得

### 高優先（Week 2）
6. `callers()` / `callees()` - 呼び出し関係
7. `each()` - イテレーション
8. `filter()` - フィルタリング
9. `get_line_info()` - 行情報

### 中優先
10. `get_switches()` - switch文
11. `get_loops()` - ループ文
12. `map()`, `find()`, `count()` - その他のメソッドチェーン
13. `header()`, `implementation()` - ファイル区分

### 低優先
14. ナビゲーション機能（`file:classes()`等）

---

## データ構造

### CallGraph（コールグラフ）

関数呼び出し関係を記録するデータ構造:

```cpp
class CallGraph {
public:
    std::unordered_map<std::string, std::vector<std::string>> callers_;  // 関数名 -> 呼び出し元
    std::unordered_map<std::string, std::vector<std::string>> callees_;  // 関数名 -> 呼び出し先

    void add_call(const std::string& caller, const std::string& callee);
    std::vector<std::string> get_callers(const std::string& func) const;
    std::vector<std::string> get_callees(const std::string& func) const;
};
```

**実装場所**: `src/engine/call_graph.hpp`, `src/engine/call_graph.cpp`

---

**この仕様書に従って実装を進めてください。不明な点があればこのドキュメントを更新してください。**
