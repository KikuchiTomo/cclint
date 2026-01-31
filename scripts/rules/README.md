# cclint Lua Rules

cclintのすべてのルールはLuaで記述されています。組み込みルールはありません。
各ルールは1ファイル1ルールの原則で細かく分割されています。

## ディレクトリ構成

### naming/ - 命名規則
細かく分割されたアクセス指定子・修飾子ごとの命名ルール

| ファイル名 | 説明 |
|-----------|------|
| `class_name_pascal_case.lua` | クラス名はPascalCase (例: MyClass) |
| `public_method_snake_case.lua` | publicメソッドはsnake_case (例: get_value) |
| `private_method_underscore_prefix.lua` | privateメソッドは_snake_case可 (例: _internal_func) |
| `protected_method_snake_case.lua` | protectedメソッドはsnake_case |
| `function_name_snake_case.lua` | グローバル関数はsnake_case |
| `private_member_trailing_underscore.lua` | privateメンバは末尾_ (例: data_) |
| `static_const_member_uppercase.lua` | static constメンバはUPPER_CASE (例: MAX_SIZE) |
| `enum_name_pascal_case.lua` | enum名はPascalCase (例: ErrorCode) |
| `enum_value_uppercase.lua` | enum値はUPPER_CASE (例: ERROR_SUCCESS) |
| `namespace_lowercase.lua` | namespace名は小文字 (例: mylib) |
| `bool_variable_prefix.lua` | bool変数は説明的な接頭辞 (is_, has_, can_) |

### restrictions/ - 制限ルール
クラス内で特定の関数や構文を禁止するルール

| ファイル名 | 説明 |
|-----------|------|
| `no_cout_in_class.lua` | クラス内でstd::cout禁止（ロガー推奨） |
| `no_printf_in_class.lua` | クラス内でprintf禁止（ロガー推奨） |
| `no_new_in_class.lua` | クラス内でnew禁止（スマートポインタ推奨） |
| `no_delete_in_class.lua` | クラス内でdelete禁止（スマートポインタ推奨） |
| `no_malloc_in_class.lua` | クラス内でmalloc/calloc/realloc禁止 |
| `no_global_using_namespace.lua` | グローバルスコープでusing namespace禁止 |
| `no_throw_in_destructor.lua` | デストラクタ内で例外throw禁止 |

### structure/ - 構造ルール
ファイル構造やinclude順序などのルール

| ファイル名 | 説明 |
|-----------|------|
| `header_guard_required.lua` | ヘッダーファイルにはヘッダーガード必須 |
| `one_class_per_file.lua` | 1ファイル1クラスの原則 |
| `include_order.lua` | include順序（標準→サードパーティ→プロジェクト） |

### style/ - スタイルルール
コードスタイルに関するルール

| ファイル名 | 説明 |
|-----------|------|
| `max_line_length.lua` | 最大行長制限（デフォルト100文字） |
| `indent_spaces_only.lua` | インデントはスペースのみ（タブ禁止） |
| `no_trailing_whitespace.lua` | 行末の空白禁止 |
| `max_consecutive_empty_lines.lua` | 連続空行の最大数制限 |

### readability/ - 可読性ルール
コードの可読性を向上させるルール

| ファイル名 | 説明 |
|-----------|------|
| `max_function_length.lua` | 関数の最大行数制限 |
| `no_magic_numbers.lua` | マジックナンバー禁止（定数化推奨） |
| `no_c_style_cast.lua` | C言語スタイルのキャスト禁止 |
| `prefer_constexpr.lua` | コンパイル時定数にはconstexpr推奨 |
| `switch_must_have_default.lua` | switch文はdefault必須 |
| `if_requires_braces.lua` | if文には{}が必要 |

## 使い方

### 個別ルールの適用

`.cclint.yaml` で使いたいルールだけを選択：

```yaml
lua_scripts:
  # 命名規則
  - path: scripts/rules/naming/class_name_pascal_case.lua
    enabled: true
    severity: warning

  - path: scripts/rules/naming/public_method_snake_case.lua
    enabled: true
    severity: warning

  # 制限
  - path: scripts/rules/restrictions/no_cout_in_class.lua
    enabled: true
    severity: error

  - path: scripts/rules/restrictions/no_new_in_class.lua
    enabled: true
    severity: warning

  # スタイル
  - path: scripts/rules/style/max_line_length.lua
    enabled: true
    severity: warning
    parameters:
      max_length: 120
```

### カテゴリ別の一括適用

特定のカテゴリのルールを全て適用したい場合は、
設定ファイルでグロブパターンが使えるなら：

```yaml
lua_scripts:
  # 命名規則全て
  - path: scripts/rules/naming/*.lua
    enabled: true
    severity: warning

  # 制限ルール全て
  - path: scripts/rules/restrictions/*.lua
    enabled: true
    severity: error
```

## ルールのカスタマイズ

### パラメータ付きルール

一部のルールはパラメータでカスタマイズ可能：

```yaml
lua_scripts:
  - path: scripts/rules/style/max_line_length.lua
    parameters:
      max_length: 80  # デフォルト: 100

  - path: scripts/rules/style/max_consecutive_empty_lines.lua
    parameters:
      max_empty_lines: 1  # デフォルト: 2

  - path: scripts/rules/readability/max_function_length.lua
    parameters:
      max_lines: 30  # デフォルト: 50
```

## Lua API リファレンス

### 基本API

#### 診断報告

```lua
cclint.report_error(line, column, message)    -- エラーを報告
cclint.report_warning(line, column, message)  -- 警告を報告
cclint.report_info(line, column, message)     -- 情報を報告
```

#### ファイルアクセス

```lua
cclint.get_file_content(file_path)  -- ファイル内容を取得
cclint.match_pattern(text, regex)   -- 正規表現マッチ
```

### クラスAPI

```lua
-- クラス一覧取得
cclint.get_classes()  -- {"ClassName1", "ClassName2", ...}

-- クラス情報取得
cclint.get_class_info(class_name)  -- {name, is_struct, line}

-- クラス詳細情報（namespace、基底クラス含む）
cclint.get_classes_with_info()
-- 戻り値: {
--   {name, namespace, qualified_name, line, is_struct, is_abstract, is_final, is_template, base_classes},
--   ...
-- }

-- 基底クラス取得
cclint.get_base_classes(class_name)
-- 戻り値: {{name, access, is_virtual}, ...}

-- 指定namespaceのクラス取得
cclint.get_classes_in_namespace(namespace_name)
-- 戻り値: {{name, line, is_struct}, ...}
```

### メソッドAPI

```lua
-- クラスのメソッド一覧
cclint.get_methods(class_name)  -- {"method1", "method2", ...}

-- メソッド情報
cclint.get_method_info(class_name, method_name)
-- 戻り値: {name, return_type, line, is_const, is_static, is_virtual, access}

-- メソッド詳細情報（パラメータ含む）
cclint.get_methods_with_info(class_name)
-- 戻り値: {
--   {name, return_type, line, column, access, is_const, is_static, is_virtual,
--    is_override, is_final, is_pure_virtual, is_noexcept, is_constexpr, parameters},
--   ...
-- }

-- アクセス指定子でフィルタリング
cclint.get_class_methods_by_access(class_name, "public")    -- publicメソッドのみ
cclint.get_class_methods_by_access(class_name, "private")   -- privateメソッドのみ
cclint.get_class_methods_by_access(class_name, "protected") -- protectedメソッドのみ

-- 全クラスの全メソッド取得
cclint.get_all_methods()
-- 戻り値: {
--   {name, class_name, namespace, return_type, line, access, is_const, is_static, is_virtual, parameters},
--   ...
-- }

-- メソッドパラメータ取得
cclint.get_function_parameters(class_name, method_name)
-- 戻り値: {{type, name}, ...}
```

### フィールドAPI

```lua
-- クラスのフィールド一覧
cclint.get_fields(class_name)  -- {"field1", "field2", ...}

-- フィールド情報
cclint.get_field_info(class_name, field_name)
-- 戻り値: {name, type, line, column, is_const, is_static, is_mutable, access, access_value}

-- アクセス指定子でフィルタリング
cclint.get_class_fields_by_access(class_name, "private")
-- 戻り値: {{name, type, line, is_const, is_static, is_mutable}, ...}
```

### 関数API

```lua
-- グローバル関数一覧
cclint.get_functions()
-- 戻り値: {{name, return_type, line, file, is_static, is_virtual, is_const}, ...}

-- 関数詳細情報
cclint.get_function_info(func_name)
-- 戻り値: {name, namespace, return_type, line, is_static, is_inline, is_constexpr, is_noexcept, parameters}

-- namespace内の関数取得
cclint.get_functions_in_namespace(namespace_name)
-- 戻り値: {{name, return_type, line}, ...}
```

### Namespace API

```lua
-- namespace一覧
cclint.get_namespaces()  -- {{name, line}, ...}

-- namespace情報
cclint.get_namespace_info(namespace_name)
-- 戻り値: {name, line, class_count, function_count}
```

### Include API

```lua
-- include一覧（基本）
cclint.get_includes()  -- {{line, text, is_system}, ...}

-- include詳細（ヘッダー名解析済み）
cclint.get_include_details()
-- 戻り値: {{line, text, header, is_system}, ...}
-- 例: {line=1, text='#include <string>', header='string', is_system=true}
```

### 制御フローAPI

```lua
-- if文一覧
cclint.get_if_statements()
-- 戻り値: {{line, has_braces, has_else}, ...}

-- ループ一覧
cclint.get_loops()
-- 戻り値: {{line, has_braces, type}, ...}  -- type: "for", "while", "do_while"

-- switch文一覧
cclint.get_switches()
-- 戻り値: {{line, has_default, case_count}, ...}
```

### 関数呼び出しAPI

```lua
-- 関数呼び出し一覧
cclint.get_function_calls()
-- 戻り値: {{function, caller, line, scope}, ...}

-- 指定関数を呼び出している関数一覧
cclint.get_callers(function_name)  -- {"caller1", "caller2", ...}

-- 指定関数が呼び出している関数一覧
cclint.get_callees(function_name)  -- {"callee1", "callee2", ...}

-- 呼び出し関係グラフ
cclint.get_call_graph()  -- {caller = {"callee1", "callee2"}, ...}

-- 呼び出し許可チェック（ルール実装用）
cclint.is_call_allowed(caller_func, called_func)  -- true/false
```

### Using宣言API

```lua
-- using宣言一覧
cclint.get_all_using_declarations()
-- 戻り値: {{line, target, alias, scope, is_global, kind}, ...}
-- kind: "namespace", "type_alias", "declaration"
```

### その他のAPI

```lua
-- enum一覧
cclint.get_enums()  -- {{name, is_class, line, values}, ...}

-- マクロ一覧
cclint.get_macros()  -- {{name, is_function, definition, line, parameters}, ...}

-- コンストラクタ/デストラクタ
cclint.get_constructors()  -- {{class_name, line, is_default, is_delete, is_explicit, access}, ...}
cclint.get_destructors()   -- {{class_name, line, is_virtual, is_default, is_delete}, ...}

-- 演算子オーバーロード
cclint.get_operators()  -- {{operator, line, is_member, is_friend}, ...}

-- テンプレート
cclint.get_templates()  -- {{name, line, is_variadic, is_specialization}, ...}

-- ラムダ式
cclint.get_lambdas()  -- {{line, capture_clause, is_mutable}, ...}

-- 継承ツリー
cclint.get_inheritance_tree()  -- {ClassName = {"BaseClass1", "BaseClass2"}, ...}

-- 属性
cclint.get_attributes()  -- {line_number = {"nodiscard", "deprecated"}, ...}

-- コメント
cclint.get_comments()  -- {{line, content, is_line_comment}, ...}
```

## ルール作成例

### 例1: 特定プレフィクスのクラスのpublicメソッドにサフィックス必須

```lua
rule_description = "Classes with 'I' prefix must have '_impl' suffix on public methods"
rule_category = "naming"

function check_ast()
    local classes = cclint.get_classes_with_info()
    for _, cls in ipairs(classes) do
        -- 'I'プレフィクスのクラス（インターフェース）をチェック
        if cls.name:match("^I[A-Z]") then
            local methods = cclint.get_class_methods_by_access(cls.name, "public")
            for _, method in ipairs(methods) do
                -- コンストラクタ、デストラクタ、演算子は除外
                if method.name ~= cls.name and not method.name:match("^~") and not method.name:match("^operator") then
                    if not method.name:match("_impl$") then
                        cclint.report_warning(method.line, 1,
                            "Public method '" .. method.name .. "' in interface '" .. cls.name ..
                            "' should end with '_impl'")
                    end
                end
            end
        end
    end
end
```

### 例2: 特定namespaceのみに適用するルール

```lua
rule_description = "Functions in 'internal' namespace must be static"
rule_category = "structure"

function check_ast()
    local funcs = cclint.get_functions_in_namespace("internal")
    for _, func in ipairs(funcs) do
        if not func.is_static then
            cclint.report_warning(func.line, 1,
                "Function '" .. func.name .. "' in 'internal' namespace should be static")
        end
    end
end
```

### 例3: privateメソッドにプレフィクス必須

```lua
rule_description = "Private methods must start with underscore"
rule_category = "naming"

function check_ast()
    local all_methods = cclint.get_all_methods()
    for _, method in ipairs(all_methods) do
        if method.access == "private" then
            -- コンストラクタ、デストラクタ、演算子は除外
            if not method.name:match("^~") and method.name ~= method.class_name and
               not method.name:match("^operator") then
                if not method.name:match("^_") then
                    cclint.report_warning(method.line, 1,
                        "Private method '" .. method.name .. "' in class '" .. method.class_name ..
                        "' should start with '_'")
                end
            end
        end
    end
end
```

### 例4: if文にはブレース必須

```lua
rule_description = "If statements must have braces"
rule_category = "style"

function check_ast()
    local if_stmts = cclint.get_if_statements()
    for _, stmt in ipairs(if_stmts) do
        if not stmt.has_braces then
            cclint.report_warning(stmt.line, 1,
                "If statement should have braces")
        end
    end
end
```

### 例5: 特定メソッドの引数パターンチェック

```lua
rule_description = "Methods starting with 'set_' must have exactly one parameter"
rule_category = "naming"

function check_ast()
    local all_methods = cclint.get_all_methods()
    for _, method in ipairs(all_methods) do
        if method.name:match("^set_") then
            local param_count = #method.parameters
            if param_count ~= 1 then
                cclint.report_warning(method.line, 1,
                    "Setter method '" .. method.name .. "' should have exactly 1 parameter, has " ..
                    param_count)
            end
        end
    end
end
```

### 例6: 特定の関数は特定の関数からのみ呼び出し可能

```lua
rule_description = "Dangerous functions can only be called from approved functions"
rule_category = "restrictions"

local dangerous_funcs = {"system", "exec", "popen"}
local approved_callers = {"safe_execute", "secure_run"}

function check_ast()
    local calls = cclint.get_function_calls()
    for _, call in ipairs(calls) do
        for _, dangerous in ipairs(dangerous_funcs) do
            if call.function == dangerous then
                local is_approved = false
                for _, approved in ipairs(approved_callers) do
                    if call.caller == approved then
                        is_approved = true
                        break
                    end
                end
                if not is_approved then
                    cclint.report_error(call.line, 1,
                        "'" .. dangerous .. "' can only be called from approved functions")
                end
            end
        end
    end
end
```

### 例7: 特定ファイルで特定ヘッダーをインクルード禁止

```lua
rule_description = "Production code should not include test headers"
rule_category = "restrictions"

local forbidden_headers = {"gtest/gtest.h", "gmock/gmock.h", "catch.hpp"}

function check_file()
    -- テストファイルは除外
    if file_path:match("_test%.cpp$") or file_path:match("_test%.hpp$") or
       file_path:match("/test/") or file_path:match("/tests/") then
        return
    end

    local includes = cclint.get_include_details()
    for _, inc in ipairs(includes) do
        for _, forbidden in ipairs(forbidden_headers) do
            if inc.header == forbidden then
                cclint.report_error(inc.line, 1,
                    "Test header '" .. forbidden .. "' should not be included in production code")
            end
        end
    end
end
```

## 重要度レベル

- **error**: ビルドを失敗させる重大な問題
- **warning**: 警告（ビルドは成功）
- **info**: 情報提供（改善提案）

## アクセス指定子の値

AST APIで返されるaccess値：
- `"public"` または `0`
- `"protected"` または `1`
- `"private"` または `2`

## ルール作成のベストプラクティス

1. **1ファイル1ルール**: 各ルールは独立したファイルに
2. **明確な説明**: rule_descriptionは具体的に
3. **適切なカテゴリ**: rule_categoryで分類
4. **パラメータ化**: 閾値などはパラメータで設定可能に
5. **テスト**: samples/でテストしてから使用

## トラブルシューティング

### ルールが適用されない

1. パスが正しいか確認
2. `enabled: true` になっているか確認
3. ファイルパターンにマッチしているか確認（include_patterns）
4. `-v` オプションで詳細ログを確認

### 誤検出がある

1. そのルールを無効化: `enabled: false`
2. または severity を下げる: `severity: info`
3. または特定のファイルを除外: exclude_patterns

## ライセンス

各ルールファイルはcclintプロジェクトのライセンスに従います。
