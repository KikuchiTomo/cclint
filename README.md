# cclint

A customizable C++ linter with Lua-scriptable rules and a complete built-in C++ parser.

## Features

cclint is a fully-featured C++ linter built from the ground up with a complete, standalone parser implementation. Unlike other linters that depend on LLVM/Clang, cclint includes its own ~6,200-line C++ parser that handles all standard C++17/20/23 syntax, from basic declarations to complex template metaprogramming.

The built-in enhanced lexer recognizes over 200 token types, including modern C++ features like the spaceship operator (<=>), UTF-8/16/32 string literals, raw string literals, and user-defined literals. The preprocessor provides full macro expansion capabilities with support for conditional compilation, variadic macros, stringification, and token pasting, all while maintaining a special "linter mode" that preserves macro names for analysis.

At its core, cclint features a complete semantic analyzer with symbol tables, type system, and scope management. This enables deep code analysis beyond simple pattern matching, allowing rules to understand the actual structure and meaning of your code. The parser handles all expression types with proper operator precedence, all control flow statements including C++17's if constexpr and range-based for loops, and provides robust error recovery to report multiple issues in a single analysis pass.

The rule system is highly flexible, offering 102 built-in rules covering naming conventions, code style, performance optimizations, and security best practices. For custom requirements, you can write rules in Lua with full access to the AST, enabling sophisticated checks tailored to your project's specific needs.

Performance is a priority: cclint uses multi-threaded analysis to process multiple files in parallel, implements SHA-256-based file caching to skip unchanged files, and supports incremental builds by tracking file dependencies through #include directives. Results can be output in multiple formats (text, JSON, XML) for seamless integration with CI/CD pipelines and development tools.

## Build

```bash
git clone https://github.com/KikuchiTomo/cclint.git
cd cclint

# Debug build (default, for development)
make build

# Release build (optimized)
make release

# Or manual build
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..  # or Release
make -j$(nproc)
```

**Requirements:**
- CMake 3.16+
- C++17 compiler (g++ or clang++)
- LuaJIT 2.1+ (optional, for Lua rule scripts)
- yaml-cpp 0.7+ (optional, for YAML configuration)

## Architecture

cclint includes a complete C++ parser implementation (~6,200 lines) with the following components:

### Enhanced Lexer
- **200+ Token Types** - Complete coverage of C++17/20/23 syntax
- **UTF-8/16/32 Support** - Full Unicode string literal handling
- **Raw String Literals** - R"(...)" syntax support
- **User-Defined Literals** - Custom literal suffixes (123_km, "hello"_s)
- **All Operators** - Including spaceship operator (<=>)

### Preprocessor
- **Macro Expansion** - Function-like and object-like macros
- **Conditional Compilation** - #if, #ifdef, #ifndef, #else, #elif, #endif
- **Include Resolution** - #include directive handling with include path search
- **Predefined Macros** - __FILE__, __LINE__, __DATE__, __TIME__, etc.
- **Variadic Macros** - __VA_ARGS__ support
- **String Operations** - # (stringification) and ## (token pasting)
- **Linter Mode** - Preserves macro names for analysis while expanding when needed

### Expression & Statement Parser
- **All Expressions** - Assignment, ternary, logical, comparison, arithmetic, unary
- **Cast Operators** - static_cast, dynamic_cast, const_cast, reinterpret_cast, C-style
- **Postfix Expressions** - Array subscript, function calls, member access (., ->)
- **Primary Expressions** - Literals, identifiers, parentheses, lambda, this
- **Control Flow** - if/if constexpr, switch, for/range-for, while, do-while
- **Exception Handling** - try-catch blocks
- **Jump Statements** - return, break, continue, goto

### Semantic Analyzer
- **Symbol Tables** - Global, namespace, class, and function scopes
- **Type System** - Built-in types, pointers, references, arrays, function types
- **User-Defined Types** - Classes, structs, enums
- **CV Qualifiers** - const, volatile, mutable
- **Type Resolution** - Automatic type inference and conversion checking
- **Name Lookup** - Local, parent scope, and qualified name resolution

### Error Recovery
- **Synchronization Points** - Recover at statement and declaration boundaries
- **Continue Parsing** - Multiple errors reported in a single pass
- **Detailed Diagnostics** - Line/column information with context

### Performance
- **Multi-threaded Analysis** - Parallel file processing
- **File Caching** - SHA-256 based content hashing
- **Incremental Builds** - Analyze only modified files
- **Dependency Tracking** - #include-based dependency graph

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

## Testing

The parser has comprehensive test coverage:

- **Expression Parser Tests** - 7/7 passed
  - Binary operators (arithmetic, logical, comparison)
  - Unary operators (++, --, !, ~, *, &)
  - Cast expressions (all C++ cast types)
  - Postfix expressions (array, function call, member access)
  - Primary expressions (literals, lambda, this)

- **Statement Parser Tests** - 5/5 passed
  - Control flow (if/if constexpr, switch, for/range-for, while, do-while)
  - Exception handling (try-catch)
  - Jump statements (return, break, continue, goto)
  - Compound statements

Run tests with:
```bash
cd build
ctest --output-on-failure
```
