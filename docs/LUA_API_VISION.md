# Lua API ビジョン - 実現したいコードの姿

**作成日**: 2026-01-28
**目的**: 新しいLua APIで実現したいコードの具体例を示す

このドキュメントでは、Lua完全移行後に書けるようになるルールの例を示します。

---

## 1. 基本的なファイル操作

### 例: 特定のファイルにのみクラスを配置できる

```lua
-- ルール: モデルクラスは models/ ディレクトリにのみ配置
function check_ast()
    local classes = cclint.get_classes()

    for _, class in ipairs(classes) do
        local info = cclint.get_class_info(class)

        if class:match("Model$") then
            -- クラス名が "Model" で終わる場合
            if not info.file:match("models/") then
                cclint.report_error(
                    info.line, 0,
                    string.format("Model class '%s' must be in models/ directory", class)
                )
            end
        end
    end
end
```

### 例: ヘッダーと実装の整合性チェック

```lua
-- ルール: ヘッダーで宣言された関数は実装ファイルに存在すべき
function check_ast()
    local files = cclint.get_files()

    for _, file in ipairs(files:header()) do
        local header_name = file:name()
        local impl_name = header_name:gsub("%.hpp$", ".cpp")

        local impl_file = files:find(function(f)
            return f:name() == impl_name
        end)

        if not impl_file then
            cclint.report_warning(
                1, 0,
                string.format("Header %s has no corresponding implementation file", header_name)
            )
        end
    end
end
```

---

## 2. クラスとメソッドのチェック

### 例: アクセス指定子に応じた命名規則

```lua
-- ルール: public = camelCase, protected = snake_case, private = _snake_case
function check_ast()
    local classes = cclint.get_classes()

    for _, class_name in ipairs(classes) do
        local methods = cclint.get_methods(class_name)

        for _, method_name in ipairs(methods) do
            local info = cclint.get_method_info(class_name, method_name)

            if info.access == "public" then
                if not method_name:match("^[a-z][a-zA-Z0-9]*$") then
                    cclint.report_warning(
                        info.line, 0,
                        string.format("Public method '%s' should use camelCase", method_name)
                    )
                end
            elseif info.access == "private" then
                if not method_name:match("^_[a-z][a-z0-9_]*$") then
                    cclint.report_warning(
                        info.line, 0,
                        string.format("Private method '%s' should start with _ and use snake_case", method_name)
                    )
                end
            end
        end
    end
end
```

### 例: getter/setterの対チェック

```lua
-- ルール: getterがあればsetterも必要
function check_ast()
    local classes = cclint.get_classes()

    for _, class_name in ipairs(classes) do
        local methods = cclint.get_methods(class_name)
        local getters = {}
        local setters = {}

        for _, method_name in ipairs(methods) do
            if method_name:match("^get[A-Z]") then
                table.insert(getters, method_name:gsub("^get", ""))
            elseif method_name:match("^set[A-Z]") then
                table.insert(setters, method_name:gsub("^set", ""))
            end
        end

        -- getterに対応するsetterがあるかチェック
        for _, property in ipairs(getters) do
            local has_setter = false
            for _, setter_prop in ipairs(setters) do
                if property == setter_prop then
                    has_setter = true
                    break
                end
            end

            if not has_setter then
                cclint.report_info(
                    1, 0,
                    string.format("Getter 'get%s' has no corresponding setter in class '%s'",
                                  property, class_name)
                )
            end
        end
    end
end
```

---

## 3. 関数呼び出し関係のチェック

### 例: main関数からのみ呼べる関数

```lua
-- ルール: init_* 関数はmain関数からのみ呼び出せる
function check_ast()
    local functions = cclint.get_functions()

    for _, func in ipairs(functions) do
        if func.name:match("^init_") then
            local callers = func:callers()

            for _, caller in ipairs(callers) do
                if caller.name ~= "main" then
                    cclint.report_error(
                        caller.line, 0,
                        string.format("Function '%s' can only be called from main(), not from '%s'",
                                      func.name, caller.name)
                    )
                end
            end
        end
    end
end
```

### 例: 特定の関数を呼んではいけない

```lua
-- ルール: productionコードでは printf を使用禁止
function check_ast()
    local functions = cclint.get_functions()

    for _, func in ipairs(functions) do
        local callees = func:callees()

        for _, callee in ipairs(callees) do
            if callee.name == "printf" then
                -- ただし、デバッグ関数内はOK
                if not func.name:match("^debug_") and not func.name:match("^test_") then
                    cclint.report_error(
                        callee.line, 0,
                        string.format("Function '%s' should not call printf(). Use logger instead.",
                                      func.name)
                    )
                end
            end
        end
    end
end
```

### 例: 循環参照の検出

```lua
-- ルール: 循環参照を検出
function check_ast()
    local functions = cclint.get_functions()

    for _, func in ipairs(functions) do
        if is_circular_call(func, {}) then
            cclint.report_error(
                func.line, 0,
                string.format("Circular call detected involving function '%s'", func.name)
            )
        end
    end
end

function is_circular_call(func, visited)
    if visited[func.name] then
        return true  -- 循環検出
    end

    visited[func.name] = true
    local callees = func:callees()

    for _, callee in ipairs(callees) do
        if is_circular_call(callee, visited) then
            return true
        end
    end

    visited[func.name] = nil
    return false
end
```

---

## 4. main関数の特別なチェック

### 例: main関数で呼べる関数を制限

```lua
-- ルール: main関数は init_*, run_*, cleanup_* のみ呼べる
function check_ast()
    local mains = cclint.get_mains()

    for _, main in ipairs(mains) do
        local callees = main:callees()

        for _, callee in ipairs(callees) do
            local name = callee.name
            if not (name:match("^init_") or
                    name:match("^run_") or
                    name:match("^cleanup_")) then
                cclint.report_warning(
                    callee.line, 0,
                    string.format("main() should only call init_*, run_*, or cleanup_* functions, not '%s'",
                                  name)
                )
            end
        end
    end
end
```

---

## 5. 制御構造のチェック

### 例: if文にブレースを強制

```lua
-- ルール: 全てのif文にブレースを付ける
function check_ast()
    local ifs = cclint.get_ifs()

    for _, if_stmt in ipairs(ifs) do
        if not if_stmt.has_braces then
            cclint.report_warning(
                if_stmt.line, 0,
                "if statement must have braces, even for single-line body"
            )
        end
    end
end
```

### 例: switch文にdefaultを強制

```lua
-- ルール: switch文には必ずdefaultを付ける
function check_ast()
    local switches = cclint.get_switches()

    for _, switch in ipairs(switches) do
        if not switch.has_default then
            cclint.report_warning(
                switch.line, 0,
                "switch statement must have a default case"
            )
        end
    end
end
```

---

## 6. enum のチェック

### 例: enum classの使用を強制

```lua
-- ルール: C++11以降では enum ではなく enum class を使用
function check_ast()
    local enums = cclint.get_enums()

    for _, enum in ipairs(enums) do
        if not enum.is_class then
            cclint.report_warning(
                enum.line, 0,
                string.format("Enum '%s' should be an enum class for type safety", enum.name)
            )
        end
    end
end
```

### 例: enum値の命名規則

```lua
-- ルール: enum値はUPPER_CASEで命名
function check_ast()
    local enums = cclint.get_enums()

    for _, enum in ipairs(enums) do
        for _, value in ipairs(enum.values) do
            if not value:match("^[A-Z][A-Z0-9_]*$") then
                cclint.report_warning(
                    enum.line, 0,
                    string.format("Enum value '%s' should be in UPPER_CASE", value)
                )
            end
        end
    end
end
```

---

## 7. マクロのチェック

### 例: マクロ関数の命名規則

```lua
-- ルール: マクロ関数はUPPER_CASEで命名
function check_ast()
    local macros = cclint.get_macros()

    for _, macro in ipairs(macros) do
        if macro.is_function then
            if not macro.name:match("^[A-Z][A-Z0-9_]*$") then
                cclint.report_warning(
                    macro.line, 0,
                    string.format("Macro function '%s' should be in UPPER_CASE", macro.name)
                )
            end
        end
    end
end
```

### 例: マクロ使用の制限

```lua
-- ルール: モダンC++ではマクロ関数を使わず、constexpr関数を使用
function check_ast()
    local macros = cclint.get_macros()

    for _, macro in ipairs(macros) do
        if macro.is_function and not macro.name:match("^ASSERT_") then
            cclint.report_info(
                macro.line, 0,
                string.format("Consider replacing macro '%s' with a constexpr function", macro.name)
            )
        end
    end
end
```

---

## 8. インデントのチェック

### 例: インデント幅の統一

```lua
-- ルール: インデント幅は4スペース
rule_description = "Check indent width is 4 spaces"
rule_category = "style"

function check_file()
    local indent_width = rule_params.width or 4

    for line_num, line in ipairs(file_lines) do
        if #line > 0 and line:match("^%s") then
            local info = cclint.get_line_info(file_path, line_num)

            if info.indent_type == "tab" then
                cclint.report_warning(
                    line_num, 1,
                    "Use spaces for indentation, not tabs"
                )
            elseif info.indent_count % indent_width ~= 0 then
                cclint.report_warning(
                    line_num, 1,
                    string.format("Indent should be a multiple of %d spaces (found %d)",
                                  indent_width, info.indent_count)
                )
            end
        end
    end
end
```

### 例: 深すぎるネストの検出

```lua
-- ルール: ネストは3レベルまで
function check_file()
    local max_indent = rule_params.max_level or 3
    local base_indent_width = 4

    for line_num, line in ipairs(file_lines) do
        if #line > 0 and not line:match("^%s*[/]") then -- コメント除外
            local info = cclint.get_line_info(file_path, line_num)
            local indent_level = math.floor(info.indent_count / base_indent_width)

            if indent_level > max_indent then
                cclint.report_warning(
                    line_num, 1,
                    string.format("Too deeply nested (level %d). Consider refactoring.", indent_level)
                )
            end
        end
    end
end
```

---

## 9. メソッドチェーンの活用

### 例: 複雑なフィルタリング

```lua
-- ルール: publicメソッドで仮想関数でないものは全てconstであるべき
function check_ast()
    cclint.get_files()
        :implementation()  -- 実装ファイルのみ
        :each(function(file)
            file:classes()
                :each(function(class)
                    class:methods()
                        :filter(function(method)
                            return method.access == "public" and
                                   not method.is_virtual and
                                   not method.is_static
                        end)
                        :each(function(method)
                            if not method.is_const then
                                cclint.report_warning(
                                    method.line, 0,
                                    string.format("Public non-virtual method '%s' should be const",
                                                  method.name)
                                )
                            end
                        end)
                end)
        end)
end
```

### 例: 統計情報の取得

```lua
-- ルール: クラスあたりのメソッド数が多すぎないかチェック
function check_ast()
    local max_methods = rule_params.max_methods or 20

    cclint.get_classes()
        :each(function(class_name)
            local method_count = cclint.get_methods(class_name):count()

            if method_count > max_methods then
                local info = cclint.get_class_info(class_name)
                cclint.report_warning(
                    info.line, 0,
                    string.format("Class '%s' has too many methods (%d). Consider splitting.",
                                  class_name, method_count)
                )
            end
        end)
end
```

---

## 10. 複雑な複合ルール

### 例: レイヤードアーキテクチャの強制

```lua
-- ルール: レイヤー間の依存関係を制限
-- UI層 -> ビジネスロジック層 -> データ層 の一方向のみ許可

local layer_hierarchy = {
    ui = 1,
    business = 2,
    data = 3
}

function get_layer(file_path)
    if file_path:match("ui/") then return "ui" end
    if file_path:match("business/") then return "business" end
    if file_path:match("data/") then return "data" end
    return nil
end

function check_ast()
    local functions = cclint.get_functions()

    for _, func in ipairs(functions) do
        local caller_layer = get_layer(func.file)

        if caller_layer then
            local callees = func:callees()

            for _, callee in ipairs(callees) do
                local callee_layer = get_layer(callee.file)

                if callee_layer and
                   layer_hierarchy[caller_layer] < layer_hierarchy[callee_layer] then
                    cclint.report_error(
                        callee.line, 0,
                        string.format(
                            "Layer violation: %s layer function '%s' cannot call %s layer function '%s'",
                            caller_layer, func.name, callee_layer, callee.name
                        )
                    )
                end
            end
        end
    end
end
```

### 例: インターフェースの実装チェック

```lua
-- ルール: Interfaceで終わるクラスは純粋仮想関数のみを持つべき
function check_ast()
    local classes = cclint.get_classes()

    for _, class_name in ipairs(classes) do
        if class_name:match("Interface$") then
            local methods = cclint.get_methods(class_name)

            for _, method_name in ipairs(methods) do
                local info = cclint.get_method_info(class_name, method_name)

                if not info.is_virtual or not info.is_pure_virtual then
                    cclint.report_error(
                        info.line, 0,
                        string.format(
                            "Interface class '%s' should only have pure virtual methods, but '%s' is not",
                            class_name, method_name
                        )
                    )
                end
            end
        end
    end
end
```

---

## 11. プロジェクト固有のルール

### 例: エラーハンドリングの強制

```lua
-- ルール: ファイルI/O関数の後には必ずエラーチェックが必要
function check_ast()
    local io_functions = {"fopen", "fread", "fwrite", "fclose"}
    local functions = cclint.get_functions()

    for _, func in ipairs(functions) do
        local callees = func:callees()

        for i, callee in ipairs(callees) do
            for _, io_func in ipairs(io_functions) do
                if callee.name == io_func then
                    -- 次の文でエラーチェックがあるか確認
                    local next_stmt = callees[i + 1]
                    if not (next_stmt and is_error_check(next_stmt)) then
                        cclint.report_error(
                            callee.line, 0,
                            string.format("Call to '%s' must be followed by error check", io_func)
                        )
                    end
                end
            end
        end
    end
end

function is_error_check(stmt)
    -- 簡易的なエラーチェック判定
    return stmt.name:match("^check_") or
           stmt.name == "if" or
           stmt.name:match("Error")
end
```

---

## まとめ

このビジョンで示したような柔軟なルールを、組み込みルールなしで全てLuaで記述できるようになります。

### 実現される主な機能
1. ✅ ファイルレベルの操作
2. ✅ クラス・メソッドの詳細なチェック
3. ✅ 関数呼び出し関係の追跡
4. ✅ main関数の特別扱い
5. ✅ 制御構造のチェック
6. ✅ enum のチェック
7. ✅ マクロのチェック
8. ✅ インデント・スタイルのチェック
9. ✅ メソッドチェーンによる直感的な操作
10. ✅ プロジェクト固有の複雑なルール

### メリット
- **完全なカスタマイズ性**: どんなルールでも実装可能
- **直感的なAPI**: メソッドチェーンで読みやすいコード
- **段階的な複雑さ**: 簡単なルールから複雑なルールまで対応
- **プロジェクト固有**: チームの独自のコーディング規約を実装可能

---

**このビジョンを実現するために、TODO_LUA_MIGRATION.md に従って実装を進めます。**
