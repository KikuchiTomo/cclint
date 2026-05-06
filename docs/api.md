# Lua API reference

cclint rules are written in Lua 5.4. Specify scripts via `[[rules]]` in
`.cclint.toml` or with the CLI flag `-r`.

## Registering a rule

```lua
cclint.register("rule-name", {
  description = "...",
  severity    = "warning",   -- "error" / "warning" / "info" / "hint"
  match       = function(node) return node.kind == "ClassDecl" end,
  check       = function(node, ctx)
    -- write your check here
  end,
})
```

`match` is optional. When omitted, `check` runs on every node.

## Reporting

| Function | Purpose |
|---|---|
| `cclint.report_error(target, msg)` | Report an error |
| `cclint.report_warn(target, msg)`  | Report a warning |
| `cclint.report_info(target, msg)`  | Report an info note |
| `cclint.report_hint(target, msg)`  | Report a hint |

`target` may be an AST node or a table of the form:

```lua
{ span = { file = "...", line = 1, column = 1, byte_start = 0, byte_end = 0 } }
```

## Node fields

| Field | Type | Description |
|---|---|---|
| `kind`              | string  | libclang `CursorKind` (e.g. `ClassDecl`, `Method`) |
| `name`              | string  | Name |
| `display_name`      | string  | Display name (with parameters) |
| `spelling`          | string  | Spelling (often equals `name`) |
| `usr`               | string  | Unified Symbol Resolution |
| `access`            | string? | `"public"` / `"protected"` / `"private"` |
| `is_definition`     | bool    | Is this a definition? |
| `is_const`          | bool    | const method |
| `is_static`         | bool    | static method |
| `is_virtual`        | bool    | virtual method |
| `is_pure_virtual`   | bool    | pure virtual method |
| `type_name`         | string? | Type spelling |
| `referenced_name`   | string? | Name of the referenced declaration (e.g., for `CallExpr`) |
| `referenced_usr`    | string? | USR of the referenced declaration |
| `included_file`     | string? | For `InclusionDirective`, the resolved file path |
| `span`              | table?  | Location (`file`, `line`, `column`, `byte_start`, `byte_end`) |
| `children`          | table   | Array of child nodes |
| `parent`            | table?  | Parent node (nil at root) |

## Common kinds

The string returned by `node.kind` may differ slightly across libclang
versions. The bundled sample rules accept multiple aliases when relevant.

| Concept | kind string |
|---|---|
| Class definition | `ClassDecl` |
| Struct definition | `StructDecl` |
| Member function | `Method` |
| Free function | `FunctionDecl` |
| Constructor | `Constructor` |
| Destructor | `Destructor` |
| Member variable | `FieldDecl` |
| Variable | `VarDecl` |
| Parameter | `ParmDecl` |
| Type alias (typedef) | `TypedefDecl` |
| Type alias (using) | `TypeAliasDecl` |
| using namespace | `UsingDirective` |
| enum definition | `EnumDecl` |
| `new` expression | `CXXNewExpr` |
| `delete` expression | `CXXDeleteExpr` |
| C-style cast | `CStyleCastExpr` |
| if statement | `IfStmt` |
| for statement | `ForStmt` |
| while statement | `WhileStmt` |
| Compound statement `{}` | `CompoundStmt` |
| TU root | `TranslationUnit` (set by cclint) |

## Example: enforce naming for `ClassA`-suffix classes

```lua
cclint.register("private-prefix-for-classA", {
  description = "private members of ClassA-suffix classes must start with private_",
  match = function(n) return n.kind == "ClassDecl" or n.kind == "StructDecl" end,
  check = function(n)
    if not n.name:match("ClassA$") then return end
    for _, child in ipairs(n.children) do
      if child.kind == "FieldDecl" and child.access == "private"
         and not child.name:match("^private_") then
        cclint.report_warn(child, string.format(
          "private member `%s` of class `%s` must start with private_",
          child.name, n.name))
      end
    end
  end,
})
```

## Include paths and compiler arguments

If standard headers (`<atomic>`, `<vector>`, ...) or local headers (`my.hpp`)
cannot be found, libclang needs to be told the right flags. Two ways:

### 1. Use `compile_commands.json` (recommended)

cclint auto-detects the file in these locations by default:

- `<root>/compile_commands.json`
- `<root>/build/`, `build-debug/`, `build-release/`, `cmake-build-*/`,
  `out/`, `out/Default/`, `target/` (each immediately under)
- Walks up to 4 levels of parent directories trying the same paths

Multiple `compile_commands.json` files are merged automatically (useful for
projects that emit one per module under `src/modules/*/compile_commands.json`).

Override via `[cdb]` in `.cclint.toml`:

```toml
[cdb]
enabled = true                                 # set false to disable entirely
path = "build-debug/compile_commands.json"     # single explicit path
paths = [                                      # multiple explicit paths
  "src/agv/compile_commands.json",
  "src/modules/foo",                           # directory means ./compile_commands.json
  "src/modules/bar",
]
search_paths = ["build", "out", "my-build"]    # directory candidates (empty = defaults)
search_parents = 4                              # parent levels to walk
walk_depth = 6                                  # tree-walk max depth (0 disables walk)
```

CLI / env:

```bash
cclint . --compile-commands path/to/compile_commands.json
CCLINT_COMPILE_COMMANDS=path/... cclint .
```

To generate via CMake: `cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..`

### 2. `extra_args` in `.cclint.toml`

```toml
extra_args = [
  "-I", "include",
  "-I", "third_party/foo/include",
  "-DENABLE_FEATURE_X=1",
  "-isystem", "/usr/include/c++/11",
]
```

Used as a fallback for files not present in `compile_commands.json`.

## Cross-TU: getting include / call relations

There is no dedicated graph API. The information is already in the AST.

- Includes: `InclusionDirective` nodes carry `included_file` (resolved path).
- Calls: `CallExpr` nodes carry `referenced_name` / `referenced_usr` for the
  callee. Use `parent` to walk up to the enclosing `FunctionDecl` / `Method`
  to get the caller.

For project-wide queries:

```lua
cclint.project_definitions(usr)  -- list of definition records for a USR
cclint.project_references(usr)   -- list of reference records for a USR
cclint.project_files()           -- list of all parsed files
```

`examples/rules/include_restriction.lua`,
`examples/rules/call_only_from_main.lua`, and
`examples/rules/no_external_internal_ref.lua` are working examples.

## Suppression

To silence specific rules in specific files or lines:

### 1. Inline source comments

```cpp
int BadName1(int x); // cclint:disable=function-snake-case

// cclint:disable-next-line=function-snake-case
int BadName2(int x);

// cclint:file-disable=class-pascal-case   // disables the rule for the whole file
class lower_case {};

int BadName3(int x); // cclint:disable                 // suppresses all rules on this line
```

Multiple rules separated by commas (`cclint:disable=rule-a,rule-b`).
Omitting `=` targets all rules.

### 2. Configuration file

`.cclint.toml`:

```toml
[[suppressions]]
files = ["src/legacy/**", "src/generated/*.cpp"]
rules = ["class-pascal-case", "function-snake-case"]

[[suppressions]]
files = ["**/_pb.cc"]
rules = ["*"]   # all rules
```

`files` are globs. Including `"*"` in `rules` matches every rule.

## Limitations

- No parallelism.
