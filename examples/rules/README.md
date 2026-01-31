# Example Lua Rules

This directory contains example Lua rules demonstrating how to create custom rules for cclint.
**Note:** cclint uses Lua for all rules - there are no built-in rules.

## Available Examples

### 1. TODO Detector (`todo_detector.lua`)

Detects TODO, FIXME, HACK, and other comment markers in source code.

**Usage:**
```yaml
lua_scripts:
  - path: examples/rules/todo_detector.lua
    priority: 100
```

**Features:**
- Detects TODO, FIXME, HACK, XXX, TEMP, BUG keywords
- Works with C++ (`//`), C (`/* */`), and Python (`#`) comment styles
- Reports as informational messages

### 2. Complexity Checker (`complexity.lua`)

Simplified cyclomatic complexity checker for functions.

**Usage:**
```yaml
lua_scripts:
  - path: examples/rules/complexity.lua
    priority: 100
    parameters:
      max_complexity: "15"
```

**Features:**
- Counts decision points (if, else, for, while, etc.)
- Configurable maximum complexity threshold
- Reports warnings for overly complex functions

**Parameters:**
- `max_complexity`: Maximum allowed complexity (default: 10)

### 3. Example Rule (`example_rule.lua`)

Comprehensive example showing various Lua rule capabilities.

**Usage:**
```yaml
lua_scripts:
  - path: examples/rules/example_rule.lua
    priority: 100
    parameters:
      strict_mode: "true"
```

**Features:**
- Line length checking
- Pattern matching (e.g., detecting `printf`)
- Include guard checking
- Parameter usage (strict mode)

## Writing Custom Lua Rules

### Basic Structure

```lua
-- Rule metadata
rule_description = "Brief description of your rule"
rule_category = "category-name"  -- e.g., "naming", "style", "security"

-- Text-based checking (line by line)
function check_file(file_path)
    for line_num, line in ipairs(file_lines) do
        -- Your rule logic here
    end
end

-- AST-based checking (recommended for semantic rules)
function check_ast()
    -- Your rule logic using AST APIs
end
```

### Available Global Variables

- `file_path`: Current file being analyzed
- `file_content`: Full file content as string
- `file_lines`: Table of individual lines (1-indexed)
- `rule_params`: Table of parameters from config file

### Available Functions

#### Diagnostic Reporting

```lua
-- Report an error (fails the build)
cclint.report_error(line_num, column, "Error message")

-- Report a warning (build continues)
cclint.report_warning(line_num, column, "Warning message")

-- Report an info message (informational only)
cclint.report_info(line_num, column, "Info message")
```

#### Utilities

```lua
-- Pattern matching (using regex)
local matched, groups = cclint.match_pattern(text, pattern)

-- Read file content
local content = cclint.get_file_content(file_path)
```

### AST APIs

cclint provides extensive AST APIs for semantic analysis. Here are the most commonly used:

#### Class APIs

```lua
-- Get all classes with full info
local classes = cclint.get_classes_with_info()
-- Returns: {name, namespace, qualified_name, line, is_struct, is_abstract, is_final, base_classes}

-- Get methods by access specifier
local public_methods = cclint.get_class_methods_by_access("MyClass", "public")
local private_methods = cclint.get_class_methods_by_access("MyClass", "private")
-- Returns: {name, return_type, line, is_const, is_static, is_virtual, parameters}

-- Get all methods from all classes
local all_methods = cclint.get_all_methods()
-- Returns: {name, class_name, namespace, return_type, line, access, parameters}

-- Get method parameters
local params = cclint.get_function_parameters("MyClass", "myMethod")
-- Returns: {{type, name}, ...}
```

#### Control Flow APIs

```lua
-- Check if statements have braces
local if_stmts = cclint.get_if_statements()
-- Returns: {line, has_braces, has_else}

-- Check loops
local loops = cclint.get_loops()
-- Returns: {line, has_braces, type}  -- type: "for", "while", "do_while"
```

#### Function Call APIs

```lua
-- Get all function calls
local calls = cclint.get_function_calls()
-- Returns: {function, caller, line, scope}

-- Get who calls a specific function
local callers = cclint.get_callers("dangerous_function")

-- Get what functions are called by a specific function
local callees = cclint.get_callees("my_function")
```

#### Include APIs

```lua
-- Get include details with parsed header name
local includes = cclint.get_include_details()
-- Returns: {line, text, header, is_system}
-- Example: {header="iostream", is_system=true}
```

#### Namespace APIs

```lua
-- Get classes in a specific namespace
local classes = cclint.get_classes_in_namespace("mylib")

-- Get functions in a specific namespace
local funcs = cclint.get_functions_in_namespace("internal")
```

For a complete API reference, see `scripts/rules/README.md`.

### Example: Simple Rule

```lua
rule_description = "Detect use of 'goto'"
rule_category = "readability"

function check_file(file_path)
    for line_num, line in ipairs(file_lines) do
        if cclint.match_pattern(line, "%s*goto%s+") then
            cclint.report_warning(
                line_num,
                1,
                "Use of 'goto' is discouraged"
            )
        end
    end
end
```

### Example: AST-based Rule

```lua
rule_description = "Private methods must start with underscore"
rule_category = "naming"

function check_ast()
    local all_methods = cclint.get_all_methods()
    for _, method in ipairs(all_methods) do
        if method.access == "private" then
            if not method.name:match("^_") and
               not method.name:match("^~") and
               method.name ~= method.class_name then
                cclint.report_warning(method.line, 1,
                    "Private method '" .. method.name .. "' should start with '_'")
            end
        end
    end
end
```

### Example: If statements require braces

```lua
rule_description = "If statements must have braces"
rule_category = "style"

function check_ast()
    local if_stmts = cclint.get_if_statements()
    for _, stmt in ipairs(if_stmts) do
        if not stmt.has_braces then
            cclint.report_warning(stmt.line, 1, "If statement should have braces")
        end
    end
end
```

### Example: Restrict function calls

```lua
rule_description = "system() can only be called from safe_execute()"
rule_category = "security"

function check_ast()
    local calls = cclint.get_function_calls()
    for _, call in ipairs(calls) do
        if call.function == "system" and call.caller ~= "safe_execute" then
            cclint.report_error(call.line, 1,
                "system() can only be called from safe_execute()")
        end
    end
end
```

### Best Practices

1. **Use Descriptive Messages**: Include context in diagnostic messages
2. **Handle Edge Cases**: Check for nil values, empty strings, etc.
3. **Performance**: Avoid complex regex in tight loops
4. **Parameters**: Make rules configurable via `rule_params`
5. **Testing**: Test rules with various code samples
6. **Prefer AST**: Use AST APIs for semantic rules instead of text matching

### Lua Pattern Syntax

Lua uses patterns (not regex). Common patterns:

- `.`: Any character
- `%s`: Whitespace
- `%w`: Alphanumeric
- `%d`: Digit
- `*`: 0 or more repetitions
- `+`: 1 or more repetitions
- `[abc]`: Character class
- `%f[%w]`: Frontier pattern (word boundary)

For full reference: https://www.lua.org/manual/5.1/manual.html#5.4.1

## Configuration Example

Complete `.cclint.yaml` with Lua rules:

```yaml
version: "1.0"
cpp_standard: "c++17"

# All rules are Lua scripts
lua_scripts:
  - path: examples/rules/todo_detector.lua
    priority: 100

  - path: examples/rules/complexity.lua
    priority: 90
    parameters:
      max_complexity: "15"

  - path: scripts/rules/naming/class_name_pascal_case.lua
    enabled: true
    severity: warning

  - path: scripts/rules/style/max_line_length.lua
    enabled: true
    severity: warning
    parameters:
      max_length: "120"

include_patterns:
  - "src/**/*.cpp"
  - "include/**/*.hpp"

exclude_patterns:
  - "third_party/**"
```

## Debugging Lua Rules

### Print Statements

```lua
print("Debug: processing " .. file_path)
```

Output will appear in cclint's log (use `-v` flag).

### Error Handling

Lua errors will be caught and reported by cclint:

```lua
function check_file(file_path)
    if not file_content then
        error("No file content available")
    end
    -- ... rest of rule
end
```

### Testing

Test your rule manually:

```bash
cclint --config=test-config.yaml -v g++ -std=c++17 test.cpp
```

## Further Reading

- [Lua 5.1 Reference Manual](https://www.lua.org/manual/5.1/)
- [Lua Rules Guide](../../scripts/rules/README.md)
- [cclint Configuration Guide](../../docs/usage.md)
