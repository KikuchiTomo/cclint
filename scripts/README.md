# cclint Standard Rules

このディレクトリには、cclintの標準Luaルールスクリプトが含まれています。

**🎊 合計100個のLuaルールスクリプトを提供！**

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

## 利用可能なルール（全100ルール）

### Naming（命名規則） - 9ルール

- **class-name-camelcase**: クラス名はCamelCaseであるべき
- **constant-name-uppercase**: 定数名はUPPER_CASEであるべき
- **enum-name-camelcase**: 列挙型名はCamelCaseであるべき
- **function-name-lowercase**: 関数名はlower_case（スネークケース）であるべき
- **member-variable-prefix**: メンバ変数には接頭辞を付けるべき
- **namespace-name-lowercase**: 名前空間名はlowercaseであるべき

### Style（スタイル） - 4ルール

- **braces-on-new-line**: 開き波括弧は新しい行に配置すべき
- **indent-width**: インデント幅を統一
- **max-line-length**: 1行の最大文字数を制限
- **space-after-control-statement**: 制御文の後にスペースを入れるべき

### Structure（構造） - 4ルール

- **forward-declaration-namespace**: 前方宣言は名前空間内で行うべき
- **header-guard**: ヘッダーファイルにはインクルードガードが必要
- **include-order**: #includeの順序を統一
- **one-class-per-file**: 1ファイルに1クラス

### Spacing（空白・改行） - 3ルール

- **max-consecutive-empty-lines**: 連続する空行の最大数を制限
- **operator-spacing**: 演算子の前後にスペースを入れるべき
- **trailing-whitespace**: 行末の空白を削除すべき

### Documentation（ドキュメント） - 3ルール

- **file-header**: ファイルヘッダーコメントが必要
- **require-function-comments**: public関数にはドキュメントコメントが必要
- **todo-comment**: TODOコメントの形式チェック

### Modernize（モダン化） - 10ルール

- **use-auto**: 型が明らかな場合はautoを使用すべき
- **use-default-member-init**: メンバ変数はデフォルトメンバ初期化を使用すべき
- **use-emplace**: push_backの代わりにemplace_backを使用すべき
- **use-equals-default**: デフォルト実装は= defaultを使用すべき
- **use-equals-delete**: 禁止すべき関数は= deleteを使用すべき
- **use-nodiscard**: 戻り値が重要な関数には[[nodiscard]]を付けるべき
- **use-noexcept**: throw()の代わりにnoexceptを使用すべき
- **use-nullptr**: NULLや0の代わりにnullptrを使用すべき
- **use-override**: 仮想関数のオーバーライドにはoverrideを使用すべき
- **use-using**: typedefの代わりにusingを使用すべき

### Performance（パフォーマンス） - 7ルール

- **avoid-unnecessary-copy**: 不要なコピーを避けるべき
- **move-const-arg**: const引数へのstd::moveは無意味
- **pass-by-const-reference**: 大きなオブジェクトはconst参照で渡すべき
- **reserve-vector**: vectorにreserve()を使用すべき
- **static-const-string**: 定数文字列はstatic constにすべき
- **unnecessary-value-param**: 変更されないパラメータはconst参照にすべき

### Readability（可読性） - 8ルール

- **avoid-nested-conditionals**: 深くネストした条件文を避けるべき
- **consistent-declaration-parameter-name**: 宣言と定義でパラメータ名を統一すべき
- **else-after-return**: return後のelseは不要
- **identifier-naming**: 識別子の長さを適切に保つべき
- **max-function-length**: 関数の長さを制限
- **no-magic-numbers**: マジックナンバーを使用すべきでない
- **simplify-boolean-expr**: 冗長なboolean式を簡略化すべき
- **switch-has-default**: switch文にはdefaultケースが必要
- **unused-parameter**: 未使用のパラメータに名前を付けるべきでない

### Security（セキュリティ） - 5ルール

- **check-array-bounds**: 配列の境界チェック
- **memset-zero-length**: memsetの長さゼロは無意味
- **no-rand**: rand()の代わりに<random>を使用すべき
- **no-system-call**: system()関数の使用を禁止
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
