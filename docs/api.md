# Lua API リファレンス

cclint のルールは Lua 5.4 で書きます．スクリプトは `.cclint.toml` の
`[[rules]]` か CLI の `-r` で指定してください．

## ルール登録

```lua
cclint.register("ルール名", {
  description = "説明",
  severity    = "warning",   -- "error" / "warning" / "info" / "hint"
  match       = function(node) return node.kind == "ClassDecl" end,
  check       = function(node, ctx)
    -- ここでチェックを書きます
  end,
})
```

`match` は省略可能です．省略すると全ノードに対して `check` が呼ばれます．

## 報告関数

| 関数 | 用途 |
|---|---|
| `cclint.report_error(target, msg)` | エラーを報告します |
| `cclint.report_warn(target, msg)`  | 警告を報告します |
| `cclint.report_info(target, msg)`  | 情報を報告します |
| `cclint.report_hint(target, msg)`  | ヒントを報告します |

`target` には AST ノードを渡してください．以下の形式の table も受け付けます．

```lua
{ span = { file = "...", line = 1, column = 1, byte_start = 0, byte_end = 0 } }
```

## ノードの属性

| 属性 | 型 | 説明 |
|---|---|---|
| `kind`              | string  | libclang の `CursorKind` (例: `ClassDecl`, `Method`) |
| `name`              | string  | 名前 |
| `display_name`      | string  | 表示名 (引数情報含む) |
| `spelling`          | string  | spelling (`name` と同じことが多い) |
| `usr`               | string  | Unified Symbol Resolution |
| `access`            | string? | `"public"` / `"protected"` / `"private"` |
| `is_definition`     | bool    | 定義かどうか |
| `is_const`          | bool    | const メソッドか |
| `is_static`         | bool    | static メソッドか |
| `is_virtual`        | bool    | 仮想関数か |
| `is_pure_virtual`   | bool    | 純粋仮想関数か |
| `type_name`         | string? | 型表記 |
| `referenced_name`   | string? | 参照先の宣言の名前 (例: `CallExpr` → 呼ばれている関数名) |
| `referenced_usr`    | string? | 参照先の USR |
| `included_file`     | string? | `InclusionDirective` の場合，解決後のファイルパス |
| `span`              | table?  | 位置情報 (`file`, `line`, `column`, `byte_start`, `byte_end`) |
| `children`          | table   | 子ノードの配列 |
| `parent`            | table?  | 親ノード (ルートのみ nil) |

## kind の例

libclang のバージョン差で名前が揺れることがあります．サンプルルールでは
両方を許容するようにしています．

| 種別 | kind 文字列 |
|---|---|
| クラス定義 | `ClassDecl` |
| 構造体定義 | `StructDecl` |
| メンバ関数 | `Method` |
| 自由関数 | `FunctionDecl` |
| コンストラクタ | `Constructor` |
| デストラクタ | `Destructor` |
| メンバ変数 | `FieldDecl` |
| 変数 | `VarDecl` |
| 引数 | `ParmDecl` |
| 型エイリアス (typedef) | `TypedefDecl` |
| 型エイリアス (using) | `TypeAliasDecl` |
| using namespace | `UsingDirective` |
| enum 定義 | `EnumDecl` |
| `new` 式 | `CXXNewExpr` |
| `delete` 式 | `CXXDeleteExpr` |
| C スタイルキャスト | `CStyleCastExpr` |
| if 文 | `IfStmt` |
| for 文 | `ForStmt` |
| while 文 | `WhileStmt` |
| 複文 `{}` | `CompoundStmt` |
| TU ルート | `TranslationUnit` (cclint 側で固定) |

## 例: ClassA suffix のクラスで private メンバの命名強制

```lua
cclint.register("private-prefix-for-classA", {
  description = "ClassA suffix のクラスの private メンバは private_ で始めること",
  match = function(n) return n.kind == "ClassDecl" or n.kind == "StructDecl" end,
  check = function(n)
    if not n.name:match("ClassA$") then return end
    for _, child in ipairs(n.children) do
      if child.kind == "FieldDecl" and child.access == "private"
         and not child.name:match("^private_") then
        cclint.report_warn(child, string.format(
          "クラス `%s` の private メンバ `%s` は private_ で始めてください",
          n.name, child.name))
      end
    end
  end,
})
```

## include / call 関係の取得

専用 API は用意しません．AST に既に含まれる情報を使ってください．

- include は `InclusionDirective` ノードの `included_file` で解決後パスが取れる．
- 呼出は `CallExpr` ノードの `referenced_name` / `referenced_usr` で呼出先が引ける．
- 呼出元の関数は `parent` を辿って `FunctionDecl` / `Method` を探す．

`examples/rules/include_restriction.lua` と `examples/rules/call_only_from_main.lua`
が実装例です．

## 制限

- 並列処理には対応していません．
