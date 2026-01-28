# cclint Lua Rules

このディレクトリには、cclintのLuaベースのルールが含まれています。
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

### 新しいルールの追加

1ファイル1ルールの原則で、簡単に追加できます：

```lua
-- my_custom_rule.lua
rule_description = "My custom rule description"
rule_category = "custom"

function check_file()
    -- テキストベースのチェック
    for line_num, line in ipairs(file_lines) do
        if line:match("pattern") then
            cclint.report_warning(line_num, 1, "Message")
        end
    end
end

function check_ast()
    -- ASTベースのチェック
    local classes = cclint.get_classes()
    -- ...
end
```

## 重要度レベル

- **error**: ビルドを失敗させる重大な問題
- **warning**: 警告（ビルドは成功）
- **info**: 情報提供（改善提案）

## アクセス指定子の値

AST APIで返されるaccess値：
- `0` = public
- `1` = protected
- `2` = private

## ルール作成のベストプラクティス

1. **1ファイル1ルール**: 各ルールは独立したファイルに
2. **明確な説明**: rule_descriptionは具体的に
3. **適切なカテゴリ**: rule_categoryで分類
4. **パラメータ化**: 閾値などはパラメータで設定可能に
5. **テスト**: samples/でテストしてから使用

## 例: アクセス指定子ごとの命名ルール

```cpp
class MyClass {
public:
    void get_value();        // ✓ public: snake_case

protected:
    void compute_result();   // ✓ protected: snake_case

private:
    void _internal_func();   // ✓ private: _snake_case可
    int data_;               // ✓ private member: trailing _
    static const int MAX_SIZE = 100;  // ✓ static const: UPPER_CASE
};
```

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
