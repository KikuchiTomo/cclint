# cclint

CCLint is A Customizable C++ Linter.

A key feature of cclint is that it does not come with predefined rules. All rules must be created and customized.
When creating arbitrary rules, there is no need to recompile cclint. Simply write a Lua script and load it.

All rules in cclint are written in Lua.
This allows you to freely define rules such as the following:

- A rule limiting specific `#include` directives to files with a particular suffix.
- A rule restricting the creation of specific objects to only within the `main` function.

It is also possible to prohibit the use of `printf` and enforce the use of a custom `logger` instead.

## Build

```bash
git clone https://github.com/KikuchiTomo/cclint.git
cd cclint
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

**Requirements:**
- CMake 3.16+
- C++17 compiler
- LuaJIT 2.1+
- yaml-cpp 0.7+

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
rule_description = "Rule description"
rule_category = "custom"

-- Text-based check
function check_file()
    for line_num, line in ipairs(file_lines) do
        if line:match("pattern") then
            cclint.report_warning(line_num, 1, "Message")
        end
    end
end

-- AST-based check
function check_ast()
    local classes = cclint.get_classes()
    for _, cls in ipairs(classes) do
        -- Check AST nodes
    end
end
```

## Lua API

### File Context
- `file_path` - Current file path
- `file_lines` - Array of lines

### Classes & Structs
- `cclint.get_classes()` - All classes/structs
- `cclint.get_class_info(name)` - Class details
- `cclint.get_methods(class_name)` - Methods in class
- `cclint.get_method_info(class, method)` - Method details
- `cclint.get_fields(class_name)` - Member variables
- `cclint.get_field_info(class, field)` - Field details
- `cclint.get_constructors()` - All constructors
- `cclint.get_constructor_info(class)` - Constructor details
- `cclint.get_destructors()` - All destructors
- `cclint.get_destructor_info(class)` - Destructor details
- `cclint.get_operators()` - Operator overloads
- `cclint.get_operator_info(operator)` - Operator details

### Functions & Variables
- `cclint.get_functions()` - All functions
- `cclint.get_variables()` - Global variables
- `cclint.get_variable_info(name)` - Variable details

### Types
- `cclint.get_enums()` - All enums
- `cclint.get_typedefs()` - All typedefs
- `cclint.get_typedef_info(name)` - Typedef details
- `cclint.get_usings()` - Using declarations
- `cclint.get_namespaces()` - All namespaces
- `cclint.get_templates()` - Template declarations
- `cclint.get_template_info(name)` - Template details

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
- `cclint.get_lambda_info(line)` - Lambda at line
- `cclint.get_friends()` - Friend declarations
- `cclint.get_static_asserts()` - Static assertions
- `cclint.get_inheritance_tree()` - Class hierarchy
- `cclint.get_attributes()` - C++ attributes

### Preprocessor
- `cclint.get_macros()` - Macro definitions
- `cclint.get_macro_info(name)` - Macro details
- `cclint.get_comments()` - Comments
- `cclint.get_includes()` - Include directives
- `cclint.get_file_info()` - File metadata

### Reporting
- `cclint.report_error(line, col, msg)`
- `cclint.report_warning(line, col, msg)`
- `cclint.report_info(line, col, msg)`

### Access Specifiers
- `0` = public
- `1` = protected
- `2` = private
