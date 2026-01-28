# cclint - Customizable C++ Linter

**cclint** はカスタマイズ可能なC++ linterです。すべてのルールをLuaスクリプトで定義できます。

## Overview

cclintはLuaベースの静的解析ツールです。C++のASTにアクセスし、独自のルールを柔軟に記述できます。

### Key Features

- Lua-based rules - すべてのルールをLuaで記述
- LuaJIT support - Luaルールの高速実行
- AST access - C++抽象構文木へのアクセス
- Fine-grained rules - 1ファイル1ルールで管理が容易
- Compiler wrapper - gcc, clang, msvcに対応
- Multiple output formats - text, JSON, XML形式で出力
- Performance - マルチスレッド対応、キャッシュ機能

## Quick Start

### Build & Install

```bash
git clone https://github.com/KikuchiTomo/cclint.git
cd cclint
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
sudo make install  # Optional
```

### Basic Usage

```bash
# Run with default config
./build/src/cclint g++ -std=c++17 main.cpp

# With custom config
./build/src/cclint --config=.cclint.yaml g++ main.cpp

# JSON output
./build/src/cclint --format=json g++ main.cpp
```

## Configuration

Create `.cclint.yaml`:

```yaml
version: "1.0"

# Select rules you want
lua_scripts:
  # Naming rules
  - path: scripts/rules/naming/class_name_pascal_case.lua
    enabled: true
    severity: warning

  - path: scripts/rules/naming/public_method_snake_case.lua
    enabled: true
    severity: warning

  # Restrictions
  - path: scripts/rules/restrictions/no_new_in_class.lua
    enabled: true
    severity: error

  # Style
  - path: scripts/rules/style/max_line_length.lua
    enabled: true
    severity: warning
    parameters:
      max_length: 100

include_patterns:
  - "*.cpp"
  - "*.hpp"
  - "src/**/*.cpp"

exclude_patterns:
  - "build/**"
  - "third_party/**"

output_format: "text"
num_threads: 0  # Auto-detect
enable_cache: true
```

## Available Rules

**30+ fine-grained rules** across 5 categories:

### Naming Rules (11 rules)
- `class_name_pascal_case.lua` - Class names: PascalCase
- `public_method_snake_case.lua` - Public methods: snake_case
- `private_method_underscore_prefix.lua` - Private methods: _snake_case
- `protected_method_snake_case.lua` - Protected methods: snake_case
- `function_name_snake_case.lua` - Functions: snake_case
- `private_member_trailing_underscore.lua` - Private members: name_
- `static_const_member_uppercase.lua` - Static const: UPPER_CASE
- `enum_name_pascal_case.lua` - Enum names: PascalCase
- `enum_value_uppercase.lua` - Enum values: UPPER_CASE
- `namespace_lowercase.lua` - Namespaces: lowercase
- `bool_variable_prefix.lua` - Booleans: is_, has_, can_

### Restriction Rules (7 rules)
- `no_cout_in_class.lua` - Prohibit std::cout (use logger)
- `no_printf_in_class.lua` - Prohibit printf (use logger)
- `no_new_in_class.lua` - Prohibit new (use smart pointers)
- `no_delete_in_class.lua` - Prohibit delete (use RAII)
- `no_malloc_in_class.lua` - Prohibit malloc/calloc/realloc
- `no_global_using_namespace.lua` - No global using namespace
- `no_throw_in_destructor.lua` - No exceptions in destructors

### Structure Rules (3 rules)
- `header_guard_required.lua` - Require header guards
- `one_class_per_file.lua` - One class per file
- `include_order.lua` - Standard include order

### Style Rules (4 rules)
- `max_line_length.lua` - Line length limit (configurable)
- `indent_spaces_only.lua` - No tabs, spaces only
- `no_trailing_whitespace.lua` - No trailing whitespace
- `max_consecutive_empty_lines.lua` - Limit empty lines

### Readability Rules (5 rules)
- `max_function_length.lua` - Function length limit
- `no_magic_numbers.lua` - Use named constants
- `no_c_style_cast.lua` - Use C++ casts
- `prefer_constexpr.lua` - Prefer constexpr over const
- `switch_must_have_default.lua` - Require default case

📚 **[View detailed rule documentation →](scripts/rules/README.md)**

## Writing Custom Rules

Create a Lua file (1 rule per file):

```lua
-- my_rule.lua
rule_description = "My custom rule"
rule_category = "custom"

function check_file()
    -- Text-based checking
    for line_num, line in ipairs(file_lines) do
        if line:match("bad_pattern") then
            cclint.report_warning(line_num, 1, "Found bad pattern")
        end
    end
end

function check_ast()
    -- AST-based checking
    local classes = cclint.get_classes()
    for _, class_name in ipairs(classes) do
        local methods = cclint.get_methods(class_name)
        -- Check methods...
    end
end
```

Add to `.cclint.yaml`:

```yaml
lua_scripts:
  - path: my_rule.lua
    enabled: true
    severity: warning
```

## Lua API

### Available Functions

**File Context:**
- `file_path` - Current file path
- `file_lines` - Array of file lines

**AST Access - Classes & Structs:**
- `cclint.get_classes()` - Get all classes and structs
- `cclint.get_class_info(name)` - Get class/struct info (includes `is_struct`, `is_union`, `is_template`, `is_abstract`, `is_final`, inheritance details, friend declarations, attributes)
- `cclint.get_methods(class_name)` - Get class methods
- `cclint.get_method_info(class, method)` - Get method details (includes all function modifiers, attributes, template info, call relationships)
- `cclint.get_fields(class_name)` - Get member variables
- `cclint.get_field_info(class, field)` - Get field details (includes type info, initializers, attributes)
- `cclint.get_constructors()` - Get all constructors
- `cclint.get_constructor_info(class)` - Get constructor details (includes `is_default`, `is_delete`, `is_explicit`, `is_constexpr`, `is_noexcept`, initializer list)
- `cclint.get_destructors()` - Get all destructors
- `cclint.get_destructor_info(class)` - Get destructor details (includes `is_virtual`, `is_default`, `is_delete`, `is_noexcept`)
- `cclint.get_operators()` - Get operator overloads
- `cclint.get_operator_info(operator)` - Get operator details (includes `is_member`, `is_friend`, parameters, return type)

**AST Access - Functions & Variables:**
- `cclint.get_functions()` - Get all functions (includes template info, attributes, modifiers)
- `cclint.get_variables()` - Get global variables (includes detailed type info, initializers, storage class)
- `cclint.get_variable_info(name)` - Get variable details (includes `is_const`, `is_static`, `is_constexpr`, `is_extern`, `is_thread_local`, `is_volatile`)

**AST Access - Types:**
- `cclint.get_enums()` - Get all enums (includes `is_class`, underlying type, attributes)
- `cclint.get_typedefs()` - Get all typedefs (includes detailed type info, attributes)
- `cclint.get_typedef_info(name)` - Get typedef details
- `cclint.get_usings()` - Get using declarations (distinguishes type aliases from using namespace)
- `cclint.get_namespaces()` - Get all namespaces
- `cclint.get_templates()` - Get template declarations (includes parameters, specialization info, variadic templates)
- `cclint.get_template_info(name)` - Get template details

**AST Access - Control Flow:**
- `cclint.get_switches()` - Get switch statements (includes `has_default`, case count)
- `cclint.get_if_statements()` - Get if statements (includes `has_braces`, `has_else`)
- `cclint.get_loops()` - Get for/while/do-while loops (includes loop type, `has_braces`)
- `cclint.get_try_statements()` - Get try-catch blocks (includes catch count)
- `cclint.get_return_statements()` - Get return statements (includes return value, `has_value`)

**AST Access - Call Relationships:**
- `cclint.get_call_graph()` - Get complete function call graph (caller → callees mapping)
- `cclint.get_function_calls()` - Get all function calls (includes caller, scope, line)
- `cclint.get_callers(function)` - Get functions that call the specified function
- `cclint.get_callees(function)` - Get functions called by the specified function

**AST Access - Advanced Features:**
- `cclint.get_lambdas()` - Get lambda expressions (includes capture clause, `is_mutable`, `is_constexpr`, return type)
- `cclint.get_lambda_info(line)` - Get lambda details at specific line
- `cclint.get_friends()` - Get friend declarations (includes kind: class/function)
- `cclint.get_static_asserts()` - Get static_assert declarations (includes condition, message)
- `cclint.get_inheritance_tree()` - Get class inheritance relationships (class → base classes mapping)
- `cclint.get_attributes()` - Get C++ attributes ([[nodiscard]], [[deprecated]], etc.)

**AST Access - Preprocessor & Comments:**
- `cclint.get_macros()` - Get macro definitions (includes `is_function`, parameters, definition)
- `cclint.get_macro_info(name)` - Get macro details
- `cclint.get_comments()` - Get all comments (includes `is_line_comment`, content)
- `cclint.get_includes()` - Get #include directives (includes `is_system` flag)
- `cclint.get_file_info()` - Get detailed file/line info (includes indentation, tabs, trailing spaces)

**Reporting:**
- `cclint.report_error(line, col, msg)` - Report error
- `cclint.report_warning(line, col, msg)` - Report warning
- `cclint.report_info(line, col, msg)` - Report info

**Access Specifier Values:**
- `0` = public
- `1` = protected
- `2` = private

## Architecture

```
cclint (Lua-First Linter)
├── CLI Parser
├── Config Loader (YAML)
├── Compiler Wrapper (gcc/clang/msvc)
├── Parser (SimpleParser + future Clang AST)
├── Lua Engine (LuaJIT)
│   ├── Lua Bridge (C++ ↔ Lua)
│   └── Rule Executor
├── Diagnostic Engine
└── Output Formatter (Text/JSON/XML)
```

## Requirements

**Runtime:**
- Ubuntu 20.04+ or macOS 11+
- LuaJIT 2.1+
- yaml-cpp 0.7+

**Build:**
- CMake 3.16+
- C++17 compiler (GCC 7+, Clang 10+)

## Examples

### Example 1: Basic Naming Check

```cpp
// main.cpp
class my_class {  // ❌ Should be PascalCase
public:
    void GetValue() {}  // ❌ Should be snake_case
};
```

```bash
$ cclint --config=naming.yaml g++ main.cpp
main.cpp:1:7: warning: Class name 'my_class' should use PascalCase
main.cpp:3:10: warning: Public method 'GetValue' should use snake_case
```

### Example 2: Restriction Check

```cpp
// bad.cpp
class MyClass {
public:
    void print() {
        std::cout << "Hello";  // ❌ No cout in classes
        int* p = new int(5);   // ❌ No new in classes
    }
};
```

```bash
$ cclint --config=restrictions.yaml g++ bad.cpp
bad.cpp:4:9: warning: Do not use std::cout in class 'MyClass'
bad.cpp:5:18: warning: Do not use 'new' in class 'MyClass'
```

## Contributing

Contributions welcome! This project uses:
- C++17 for core engine
- LuaJIT for rules
- CMake for building

---

**Note**: cclint is a Lua-first linter. All rules are in Lua. No hardcoded C++ rules.
