# cclint Sample Project - Usage Guide

## Quick Start

```bash
# Run cclint on all files
make lint

# Or run manually on specific files
../../build/src/cclint --config .cclint.yaml /bin/true include/*.hpp
../../build/src/cclint --config .cclint.yaml /bin/true src/*.cpp
../../build/src/cclint --config .cclint.yaml /bin/true target/*.cpp
```

## Expected Output

### bad_naming.hpp (11 errors)
```
include/bad_naming.hpp:5:1: error: Class name 'WrongName' does not match filename 'bad_naming'. Expected class name: 'BadNaming'
include/bad_naming.hpp:10:1: error: Public method 'SetValue' must use lower_snake_case naming convention
include/bad_naming.hpp:11:1: error: Public method 'GetValue' must use lower_snake_case naming convention
include/bad_naming.hpp:15:1: error: Private method 'calculateInternal' must start with '__' (double underscore)
include/bad_naming.hpp:15:1: error: Private method 'calculateInternal' must use lower_snake_case naming convention
include/bad_naming.hpp:18:1: error: Private method 'UpdateState' must start with '__' (double underscore)
include/bad_naming.hpp:18:1: error: Private method 'UpdateState' must use lower_snake_case naming convention
include/bad_naming.hpp:21:1: error: Member variable 'wrongValue' must end with '_' (underscore)
include/bad_naming.hpp:21:1: error: Member variable 'wrongValue' must use lower_snake_case naming convention
include/bad_naming.hpp:22:1: error: Member variable 'badData' must end with '_' (underscore)
include/bad_naming.hpp:22:1: error: Member variable 'badData' must use lower_snake_case naming convention
```

### multiple_classes.cpp (3 errors)
```
src/multiple_classes.cpp:5:1: error: Class name 'FirstClass' does not match filename 'multiple_classes'. Expected class name: 'MultipleClasses'
src/multiple_classes.cpp:16:1: error: Class name 'SecondClass' does not match filename 'multiple_classes'. Expected class name: 'MultipleClasses'
src/multiple_classes.cpp:16:1: error: Multiple classes in one file (found 2 classes). Each .cpp file should contain exactly one class.
```

### call_graph_test.cpp (2 errors)
```
target/call_graph_test.cpp:15:1: error: Function 'only_initialize_system' should only be called from main(), but is called from 'helper_function'
target/call_graph_test.cpp:25:1: error: Function 'only_cleanup' should only be called from main(), but is called from 'bad_function'
```

### calculator.hpp + calculator.cpp (0 errors)
Good example following all naming conventions!

## Lua AST APIs Demonstrated

### Class Analysis
- `cclint.get_classes_with_info()` - Get all classes with detailed info (name, namespace, line, is_struct, etc.)
- `cclint.get_class_methods_by_access(class_name, "public"|"private"|"protected")` - Get methods filtered by access
- `cclint.get_class_fields_by_access(class_name, "public"|"private"|"protected")` - Get fields filtered by access

### Function Analysis
- `cclint.get_functions()` - Get all function names
- `cclint.get_function_info(function_name)` - Get function details (line, return_type, parameters, etc.)
- `cclint.get_function_calls()` - Get all function calls with caller/callee information

### File Information
- `cclint.get_file_info()` - Get current file path and line information

### Reporting
- `cclint.report_error(line, column, message)` - Report an error
- `cclint.report_warning(line, column, message)` - Report a warning
- `cclint.report_info(line, column, message)` - Report info

## Customizing Rules

All rules are in `.cclint/rules/` directory. You can:

1. **Modify existing rules**: Edit any `.lua` file
2. **Add new rules**: Create a new `.lua` file and add it to `.cclint.yaml`
3. **Disable rules**: Comment out or remove from `.cclint.yaml`

Example new rule:
```lua
-- .cclint/rules/my_custom_rule.lua
rule_description = "My custom rule description"
rule_category = "custom"

function check_ast()
    local classes = cclint.get_classes_with_info()
    for _, class_info in ipairs(classes) do
        -- Your custom logic here
    end
end
```

Then add to `.cclint.yaml`:
```yaml
lua_scripts:
  - path: .cclint/rules/my_custom_rule.lua
    enabled: true
    severity: error
    priority: 100
```

## Rule Explanations

1. **one_class_per_file** - Enforces single responsibility principle
2. **filename_matches_class** - Makes code navigation easier
3. **called_only_from_main** - Enforces function call restrictions
4. **private_method_prefix** - Visual distinction for private methods
5-7. **snake_case rules** - Consistent naming convention
8. **member_variable_suffix** - Distinguishes member variables from locals
9. **function_max_lines** - Prevents overly complex functions

## Build and Run

```bash
# Build the sample projects
make

# Run executables
make run

# Clean
make clean

# Lint (run cclint)
make lint
```
