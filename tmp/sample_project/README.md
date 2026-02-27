# cclint Sample Project

This is a sample C++ project demonstrating the use of cclint with custom Lua rules.

## Project Structure

```
sample_project/
├── include/           # Header files (.hpp)
│   ├── calculator.hpp (good example)
│   └── bad_naming.hpp (violates naming conventions)
├── src/               # Source files (.cpp)
│   ├── calculator.cpp (good example)
│   ├── bad_naming.cpp (violates naming conventions)
│   └── multiple_classes.cpp (violates "one class per file" rule)
├── target/            # Executable targets (each contains main())
│   ├── calculator_demo.cpp
│   ├── bad_naming_demo.cpp
│   └── call_graph_test.cpp
├── .cclint/           # cclint configuration
│   ├── rules/         # Custom Lua rules
│   │   ├── one_class_per_file.lua
│   │   ├── filename_matches_class.lua
│   │   ├── called_only_from_main.lua
│   │   ├── private_method_prefix.lua
│   │   ├── private_method_snake_case.lua
│   │   ├── public_method_snake_case.lua
│   │   ├── member_variable_snake_case.lua
│   │   ├── member_variable_suffix.lua
│   │   └── function_max_lines.lua
│   └── config.lua
├── .cclint.yaml       # Main cclint configuration
├── Makefile           # Build system
└── README.md          # This file
```

## Custom Lua Rules

This project demonstrates 9 custom Lua rules:

1. **one_class_per_file.lua**: Each .cpp file should contain exactly one top-level class
2. **filename_matches_class.lua**: Filename should match class name (snake_case → PascalCase)
3. **called_only_from_main.lua**: Functions starting with "only_" should only be called from main()
4. **private_method_prefix.lua**: Private methods must start with '__' (double underscore)
5. **private_method_snake_case.lua**: Private methods must use lower_snake_case
6. **public_method_snake_case.lua**: Public methods must use lower_snake_case
7. **member_variable_snake_case.lua**: Member variables must use lower_snake_case
8. **member_variable_suffix.lua**: Member variables must end with '_' (underscore)
9. **function_max_lines.lua**: Functions must not exceed 1500 lines

## Building the Project

```bash
# Build all targets
make

# Clean build artifacts
make clean

# Run all executables
make run
```

## Running cclint

```bash
# Run cclint on the entire project
make lint

# Or run cclint manually
../../build/src/cclint --config .cclint.yaml include/*.hpp
../../build/src/cclint --config .cclint.yaml src/*.cpp
../../build/src/cclint --config .cclint.yaml target/*.cpp
```

## Expected Violations

### good_example.cpp / calculator.hpp / calculator.cpp
- ✅ No violations (follows all naming conventions)

### bad_naming.hpp / bad_naming.cpp
- ❌ Class name 'WrongName' does not match filename 'bad_naming' (should be 'BadNaming')
- ❌ Public method 'SetValue' must use lower_snake_case
- ❌ Public method 'GetValue' must use lower_snake_case
- ❌ Private method 'calculateInternal' must start with '__'
- ❌ Private method 'calculateInternal' must use lower_snake_case (after prefix)
- ❌ Private method 'UpdateState' must start with '__'
- ❌ Private method 'UpdateState' must use lower_snake_case
- ❌ Member variable 'wrongValue' must end with '_'
- ❌ Member variable 'wrongValue' must use lower_snake_case
- ❌ Member variable 'badData' must end with '_'
- ❌ Member variable 'badData' must use lower_snake_case

### multiple_classes.cpp
- ❌ Multiple classes in one file (found 2 classes)

### call_graph_test.cpp
- ❌ Function 'only_initialize_system' should only be called from main(), but is called from 'helper_function'
- ❌ Function 'only_cleanup' should only be called from main(), but is called from 'bad_function'

## Learning Points

1. **AST-based Linting**: These rules use cclint's AST API (`cclint.get_classes()`, `cclint.get_methods()`, etc.)
2. **Call Graph Analysis**: The call graph rule uses `cclint.get_function_calls()` to track function calls
3. **Naming Conventions**: Multiple rules enforce consistent naming (snake_case, PascalCase, prefixes, suffixes)
4. **Project Structure**: Rules can enforce project organization (one class per file)
5. **Customizability**: All rules are written in Lua and can be easily modified or extended

## Modifying Rules

To modify a rule:
1. Edit the Lua file in `.cclint/rules/`
2. Run `make lint` again (no rebuild needed!)

To add a new rule:
1. Create a new Lua file in `.cclint/rules/`
2. Add it to `.cclint.yaml` under `lua_scripts`
3. Run `make lint`
