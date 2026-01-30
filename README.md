# cclint

A customizable C++ linter with Lua-scriptable rules.

## Why cclint?

Want to add custom linting rules to your C++ project without the complexity of building against large frameworks? cclint lets you write rules in Lua.

- Write custom rules in Lua with full AST access
- No recompilation - edit Lua scripts and run
- Lightweight standalone tool
- Fast - multi-threaded analysis with file caching

## Quick Start

```bash
# Build
git clone https://github.com/KikuchiTomo/cclint.git
cd cclint
make build

# Run with custom rule
./build/src/cclint --config=.cclint.yaml g++ main.cpp
```

## Build Requirements

- CMake 3.16+
- C++17 compiler (g++ or clang++)
- LuaJIT 2.1+ (optional, for Lua rule scripts)
- yaml-cpp 0.7+ (optional, for YAML configuration)

## Usage

```bash
# Basic
./build/src/cclint g++ main.cpp

# With config
./build/src/cclint --config=.cclint.yaml g++ main.cpp

# JSON output
./build/src/cclint --format=json g++ main.cpp
```

## Configuration

`.cclint.yaml`:

```yaml
version: "1.0"

lua_scripts:
  - path: path/to/rule.lua
    priority: 100

include_patterns:
  - "*.cpp"
  - "*.hpp"

exclude_patterns:
  - "build/**"

output_format: "text"
num_threads: 0
enable_cache: true
```

## Writing Rules

`my_rule.lua`:

```lua
rule_description = "Check class naming convention"
rule_category = "naming"

function check_ast()
    local classes = cclint.get_classes()
    for _, cls in ipairs(classes) do
        if not cls.name:match("^[A-Z]") then
            cclint.report_warning(cls.line, 1,
                "Class name should start with uppercase: " .. cls.name)
        end
    end
end
```

## Lua API

### Classes & Structs
- `cclint.get_classes()` - All classes/structs
- `cclint.get_class_info(name)` - Class details
- `cclint.get_methods(class_name)` - Methods in class
- `cclint.get_method_info(class, method)` - Method details
- `cclint.get_fields(class_name)` - Member variables
- `cclint.get_field_info(class, field)` - Field details
- `cclint.get_constructors()` - All constructors
- `cclint.get_destructor_info(class)` - Destructor details
- `cclint.get_operators()` - Operator overloads

### Functions & Variables
- `cclint.get_functions()` - All functions
- `cclint.get_variables()` - Global variables
- `cclint.get_variable_info(name)` - Variable details

### Types
- `cclint.get_enums()` - All enums
- `cclint.get_typedefs()` - All typedefs
- `cclint.get_usings()` - Using declarations
- `cclint.get_namespaces()` - All namespaces
- `cclint.get_templates()` - Template declarations

### Control Flow
- `cclint.get_switches()` - Switch statements
- `cclint.get_if_statements()` - If statements
- `cclint.get_loops()` - Loops
- `cclint.get_try_statements()` - Try-catch blocks
- `cclint.get_return_statements()` - Return statements

### Call Graph
- `cclint.get_call_graph()` - Function call graph
- `cclint.get_function_calls()` - All function calls
- `cclint.get_callers(function)` - Functions calling target
- `cclint.get_callees(function)` - Functions called by target

### Advanced
- `cclint.get_lambdas()` - Lambda expressions
- `cclint.get_friends()` - Friend declarations
- `cclint.get_inheritance_tree()` - Class hierarchy
- `cclint.get_attributes()` - C++ attributes

### Preprocessor
- `cclint.get_macros()` - Macro definitions
- `cclint.get_macro_info(name)` - Macro details
- `cclint.get_comments()` - Comments
- `cclint.get_includes()` - Include directives

### Reporting
- `cclint.report_error(line, col, msg)`
- `cclint.report_warning(line, col, msg)`
- `cclint.report_info(line, col, msg)`

### File Context
- `file_path` - Current file path
- `file_lines` - Array of lines

## Testing

```bash
cd build
ctest --output-on-failure
```

## License

See LICENSE file for details.
