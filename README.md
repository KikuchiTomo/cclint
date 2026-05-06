# cclint

**Customizable C++ Linter** — libclang による AST 解析と Lua スクリプトによる
完全カスタマイズ可能なルール記述を組み合わせた C++ 用 linter。

## 特徴

- **Rust 製**, libclang を内部組み込み (FFI でリンク)。
- **AST 全要素にアクセスできる Lua API**。`cclint.register` で任意のルールを定義。
- **デフォルトでルールゼロ**。必要なものだけ書く / 配る。
- **日本語ルール名・日本語メッセージ**完全対応。
- C++ 構文エラーは clang の診断としてそのまま整形表示。

## 必要なもの

- Rust (stable, 1.75+)
- libclang (`brew install llvm` または apt の `libclang-dev` 等)
- C 開発ツールチェーン

## ビルド & 実行

```bash
make build      # リリースビルド
make run        # tests/cpp_fixtures に対して example ルール実行
make test       # cargo test + 違反検出 e2e テスト
make install    # /usr/local/bin/cclint にインストール
make clean
```

## 使い方

`.cclint.toml` をプロジェクト直下に置く:

```toml
cpp_standard = "c++17"
include_patterns = ["src/**/*.cpp", "src/**/*.h"]
exclude_patterns = ["build/**"]

[[rules]]
path = "rules/private_prefix.lua"

[[rules]]
path = "rules/forbid_global_new.lua"
```

`cclint .` で実行。`--format json` で機械可読出力。

## ルールの書き方 (Lua)

```lua
cclint.register("ルール名 (日本語OK)", {
  description = "説明",
  severity = "warning",   -- error / warning / info / hint
  match = function(node) return node.kind == "ClassDecl" end,
  check = function(node, ctx)
    if not node.name:match("ClassA$") then return end
    for _, child in ipairs(node.children) do
      if child.kind == "FieldDecl" and child.access == "private"
         and not child.name:match("^private_") then
        cclint.report_warn(child, "private_ プレフィックスが必要です")
      end
    end
  end,
})
```

### 公開 API

| 関数 | 用途 |
|---|---|
| `cclint.register(name, def)` | ルール登録 |
| `cclint.report_error(target, msg)` | エラーを報告 |
| `cclint.report_warn(target, msg)`  | 警告を報告 |
| `cclint.report_info(target, msg)`  | 情報を報告 |
| `cclint.report_hint(target, msg)`  | ヒントを報告 |

`target` は AST ノード or `{ span = { file=, line=, column=, byte_start=, byte_end= } }`。

### ノード属性

```
kind, name, display_name, spelling, usr,
access ("public"/"protected"/"private"),
is_definition, is_const, is_static, is_virtual, is_pure_virtual,
type_name, span = {file, line, column, byte_start, byte_end},
children = [ ... ]
```

`kind` は libclang の `CursorKind` (`ClassDecl`, `FieldDecl`, `FunctionDecl`,
`CXXMethodDecl`, `CXXNewExpr`, `CXXDeleteExpr` など) をそのまま文字列化したもの。

## サンプルルール (`examples/rules/`)

| ファイル | 概要 |
|---|---|
| `private_prefix.lua`              | 特定 suffix のクラスで `private_` 命名強制 |
| `forbid_global_new.lua`           | 素の `new` 禁止 |
| `header_pragma_once.lua`          | ヘッダに `#pragma once` 必須 |
| `lifo_new_delete.lua`             | `new`/`delete` の LIFO 順序チェック (雛形) |
| `class_pascal_case.lua`           | クラス/構造体名は PascalCase |
| `function_snake_case.lua`         | 自由関数は snake_case |
| `no_using_namespace_in_header.lua`| ヘッダで `using namespace` 禁止 |
| `no_c_style_cast.lua`             | C スタイルキャスト禁止 |
| `forbid_plain_enum.lua`           | 素の `enum` 禁止 (`enum class` 必須) |
| `explicit_single_arg_ctor.lua`    | 単一引数コンストラクタは `explicit` 必須 |
| `virtual_destructor.lua`          | 仮想関数を持つクラスは virtual デストラクタ必須 |
| `max_function_params.lua`         | 引数 5 個まで |
| `no_global_variable.lua`          | グローバル変数禁止 (const/constexpr 除く) |
| `no_typedef.lua`                  | `typedef` 禁止 (`using` 推奨) |
| `require_braces.lua`              | `if/else/for/while` 本体は必ず `{}` で囲む |

## ライセンス

Apache-2.0
