# cclint Standard Rules

このディレクトリには、cclintの標準Luaルールスクリプトが含まれています。

## ディレクトリ構成

```
scripts/rules/
├── naming/          # 命名規則
├── style/           # コーディングスタイル
├── structure/       # コード構造
├── spacing/         # 空白・改行
├── documentation/   # ドキュメント
├── modernize/       # モダンC++への移行
├── performance/     # パフォーマンス
├── readability/     # 可読性
└── security/        # セキュリティ
```

## ルールの使用方法

### 設定ファイルでの有効化

`.cclint.yaml`ファイルでルールを有効化します：

```yaml
rules:
  - name: class-name-camelcase
    enabled: true
    severity: warning

  - name: function-name-lowercase
    enabled: true
    severity: warning
    parameters:
      allow_camelcase: false
```

### Luaスクリプトとして直接指定

カスタムLuaスクリプトを使用する場合：

```yaml
lua_scripts:
  - path: scripts/rules/naming/class_name_camelcase.lua
    priority: 100
    parameters:
      allow_underscores: false
```

## 利用可能なルール

### Naming（命名規則）

- **class-name-camelcase**: クラス名はCamelCaseであるべき
- **function-name-lowercase**: 関数名はlower_case（スネークケース）であるべき
- **constant-name-uppercase**: 定数名はUPPER_CASEであるべき

### Style（スタイル）

- **braces-on-new-line**: 開き波括弧は新しい行に配置すべき

### Structure（構造）

- **one-class-per-file**: 1ファイルに1クラス

### Spacing（空白・改行）

- **max-consecutive-empty-lines**: 連続する空行の最大数を制限

### Documentation（ドキュメント）

- **require-function-comments**: public関数にはドキュメントコメントが必要

### Modernize（モダン化）

- **use-nullptr**: NULLや0の代わりにnullptrを使用すべき
- **use-auto**: 型が明らかな場合はautoを使用すべき

### Performance（パフォーマンス）

- **pass-by-const-reference**: 大きなオブジェクトはconst参照で渡すべき

### Readability（可読性）

- **max-function-length**: 関数の長さを制限

### Security（セキュリティ）

- **no-unsafe-functions**: 安全でない関数の使用を禁止

## ルールのカスタマイズ

各ルールはパラメータでカスタマイズ可能です。詳細は各Luaスクリプトのヘッダーコメントを参照してください。

## 新しいルールの作成

独自のルールを作成する場合は、既存のルールを参考にしてください。

基本的な構造：

```lua
local rule = {
    name = "my-rule",
    description = "ルールの説明",
    severity = "warning",
}

function rule:init(params)
    -- パラメータの初期化
end

function rule:visit_function_decl(func_decl)
    -- 関数宣言を訪問した際の処理
end

return rule
```

## Lua API

cclintは100以上のLua APIを提供します。詳細は`docs/lua_api.md`を参照してください。
