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

## include パス・コンパイラ引数

C++ ヘッダ (`<atomic>`, `<vector>`, ...) や独自ヘッダ (`my.hpp`) が見つからない
場合，libclang にコンパイラ引数を渡す必要があります．方法は 2 つ．

### 1. `compile_commands.json` を使う (推奨)

デフォルトで以下の場所を自動検出します:

- `<root>/compile_commands.json`
- `<root>/build/`，`build-debug/`，`build-release/`，`cmake-build-*/`，
  `out/`，`out/Default/`，`target/` (各ディレクトリ直下)
- 親ディレクトリへ最大 4 階層さかのぼって同じ場所を試す

明示指定や挙動の調整は `.cclint.toml` の `[cdb]` セクションで:

```toml
[cdb]
enabled = true                                 # false で完全に無効化
path = "build-debug/compile_commands.json"     # 明示パス (相対 or 絶対)
search_paths = ["build", "out", "my-build"]    # 探索ディレクトリ (空ならデフォルト)
search_parents = 4                              # 親ディレクトリ何階層まで
```

CLI でも指定可:
```bash
cclint . --compile-commands path/to/compile_commands.json
# 環境変数でも可
CCLINT_COMPILE_COMMANDS=path/... cclint .
```

CMake で生成するなら: `cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..`

### 2. `.cclint.toml` の `extra_args` で手動指定

```toml
extra_args = [
  "-I", "include",
  "-I", "third_party/foo/include",
  "-DENABLE_FEATURE_X=1",
  "-isystem", "/usr/include/c++/11",
]
```

`compile_commands.json` が無いファイルにのみ適用されます．

## 抑制 (suppression)

特定のファイル・行・ルールだけ警告を抑えたいとき:

### 1. ソースのコメントで指定

```cpp
int BadName1(int x); // cclint:disable=function-snake-case

// cclint:disable-next-line=function-snake-case
int BadName2(int x);

// cclint:file-disable=class-pascal-case   ← ファイル全体で対象ルールを抑制
class lower_case {};

int BadName3(int x); // cclint:disable                 ← その行の全ルール抑制
```

カンマ区切りで複数指定可能 (`cclint:disable=rule-a,rule-b`)．`=` を省くと
全ルール対象．

### 2. 設定ファイルで指定

`.cclint.toml`:

```toml
[[suppressions]]
files = ["src/legacy/**", "src/generated/*.cpp"]
rules = ["class-pascal-case", "function-snake-case"]

[[suppressions]]
files = ["**/_pb.cc"]
rules = ["*"]   # 全ルール
```

`files` は glob．`rules` に `"*"` を含めると全ルール対象．

## 制限

- 並列処理には対応していません．
