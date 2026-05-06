# サンプルルール

`examples/rules/` 以下に同梱しています．必要なものだけ `.cclint.toml` で
有効化してください．デフォルトでは何も有効になっていません．

| ファイル | 概要 |
|---|---|
| `private_prefix.lua`               | ClassA suffix のクラスの private メンバに `private_` を強制します |
| `forbid_global_new.lua`            | 素の `new` を警告します |
| `header_pragma_once.lua`           | ヘッダに `#pragma once` を要求します |
| `lifo_new_delete.lua`              | `new`/`delete` の LIFO 順序チェックの雛形です |
| `class_pascal_case.lua`            | クラス・構造体名を PascalCase にします |
| `function_snake_case.lua`          | 自由関数を snake_case にします |
| `no_using_namespace_in_header.lua` | ヘッダ内の `using namespace` を禁止します |
| `no_c_style_cast.lua`              | C スタイルキャストを禁止します |
| `forbid_plain_enum.lua`            | 素の `enum` を禁止し，`enum class` を要求します |
| `explicit_single_arg_ctor.lua`     | 単一引数のコンストラクタに `explicit` を要求します |
| `virtual_destructor.lua`           | 仮想関数を持つクラスに virtual デストラクタを要求します |
| `max_function_params.lua`          | 関数の引数を 5 個までに制限します |
| `no_global_variable.lua`           | グローバルなミュータブル変数を禁止します |
| `no_typedef.lua`                   | `typedef` を禁止し，`using` を要求します |
| `require_braces.lua`               | `if`/`else`/`for`/`while` の本体に `{}` を要求します |
| `include_restriction.lua`          | `_internal.h` は対応する `_internal.cpp` からのみ include 可とします |
| `call_only_from_main.lua`          | `init_secret` は main からのみ呼出し可とします |

## 設定例

```toml
cpp_standard = "c++17"
include_patterns = ["src/**/*.cpp", "src/**/*.h"]

[[rules]]
path = "examples/rules/class_pascal_case.lua"

[[rules]]
path = "examples/rules/require_braces.lua"
```
