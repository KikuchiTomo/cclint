# Example Lua Rules

This directory contains example Lua rules demonstrating how to create custom rules for cclint.

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

-- Main checking function
function check_file(file_path)
    -- Your rule logic here
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
-- Report an error
cclint.report_error(line_num, column, "Error message")

-- Report a warning
cclint.report_warning(line_num, column, "Warning message")

-- Report an info message
cclint.report_info(line_num, column, "Info message")
```

#### Utilities

```lua
-- Pattern matching (Lua patterns)
local matched, groups = cclint.match_pattern(text, pattern)

-- Read file content
local content = cclint.get_file_content(file_path)
```

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

### Best Practices

1. **Use Descriptive Messages**: Include context in diagnostic messages
2. **Handle Edge Cases**: Check for nil values, empty strings, etc.
3. **Performance**: Avoid complex regex in tight loops
4. **Parameters**: Make rules configurable via `rule_params`
5. **Testing**: Test rules with various code samples

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

# Built-in rules
rules:
  - name: naming-convention
    enabled: true
    severity: warning

# Lua script rules
lua_scripts:
  - path: examples/rules/todo_detector.lua
    priority: 100

  - path: examples/rules/complexity.lua
    priority: 90
    parameters:
      max_complexity: "15"

  - path: examples/rules/example_rule.lua
    priority: 80
    parameters:
      strict_mode: "true"

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
- [cclint Configuration Guide](../docs/usage.md)
- [Lua API Reference](../docs/lua_api.md)
