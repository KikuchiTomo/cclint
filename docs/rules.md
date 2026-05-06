# Sample rules

Bundled under `examples/rules/`. Enable only what you need in `.cclint.toml`;
nothing is on by default.

| File | Description |
|---|---|
| `private_prefix.lua`               | Enforce `private_` prefix on private members of `ClassA`-suffix classes |
| `forbid_global_new.lua`            | Warn on bare `new` |
| `header_pragma_once.lua`           | Require `#pragma once` in headers |
| `lifo_new_delete.lua`              | Skeleton check for LIFO order of `new`/`delete` |
| `class_pascal_case.lua`            | Class / struct names must be PascalCase |
| `function_snake_case.lua`          | Free functions must be snake_case |
| `no_using_namespace_in_header.lua` | Forbid `using namespace` in headers |
| `no_c_style_cast.lua`              | Forbid C-style casts |
| `forbid_plain_enum.lua`            | Forbid plain `enum`; require `enum class` |
| `explicit_single_arg_ctor.lua`     | Single-argument constructors must be `explicit` |
| `virtual_destructor.lua`           | Classes with virtual functions must have a virtual destructor |
| `max_function_params.lua`          | Limit function parameters to 5 |
| `no_global_variable.lua`           | Forbid mutable global variables |
| `no_typedef.lua`                   | Forbid `typedef`; require `using` |
| `require_braces.lua`               | Require braces around `if`/`else`/`for`/`while` bodies |
| `include_restriction.lua`          | `_internal.h` may only be included from the matching `_internal.cpp` |
| `call_only_from_main.lua`          | `init_secret` may only be called from `main` |
| `no_external_internal_ref.lua`    | Symbols whose name contains `_internal_` cannot be referenced outside their defining file |

## Configuration example

```toml
cpp_standard = "c++17"
include_patterns = ["src/**/*.cpp", "src/**/*.h"]

[[rules]]
path = "examples/rules/class_pascal_case.lua"

[[rules]]
path = "examples/rules/require_braces.lua"
```
