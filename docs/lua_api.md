# Lua API Reference

This document describes the Lua API available for writing custom cclint rules.

## Overview

cclint provides a Lua API that allows you to write custom linting rules in Lua. The API is exposed through the global `cclint` table.

## Prerequisites

- LuaJIT 2.1+ (cclint is built with conditional LuaJIT support)
- Basic understanding of Lua programming
- Familiarity with C++ syntax and structure

## Global Variables

When your rule's `check_file` function is called, the following global variables are available:

### `file_path`
**Type:** `string`

The path to the file being analyzed.

```lua
print("Analyzing: " .. file_path)
```

### `file_content`
**Type:** `string`

The complete content of the file as a single string.

```lua
local line_count = select(2, file_content:gsub("\n", "\n")) + 1
print("File has " .. line_count .. " lines")
```

### `file_lines`
**Type:** `table` (array of strings)

The file content split into lines. Lines are 1-indexed.

```lua
for line_num, line in ipairs(file_lines) do
    print(line_num .. ": " .. line)
end
```

### `rule_params`
**Type:** `table` (key-value pairs)

Parameters passed from the configuration file.

```lua
local max_length = 80
if rule_params and rule_params.max_length then
    max_length = tonumber(rule_params.max_length)
end
```

### Metadata Variables

These variables should be set at the top level of your script:

#### `rule_description`
**Type:** `string`

Brief description of what your rule checks.

```lua
rule_description = "Checks for TODO comments in code"
```

#### `rule_category`
**Type:** `string`

Category of the rule (e.g., "naming", "style", "security").

```lua
rule_category = "documentation"
```

## API Functions

All API functions are accessed through the global `cclint` table.

### Diagnostic Reporting

#### `cclint.report_error(line, column, message)`

Reports an error diagnostic.

**Parameters:**
- `line` (number): Line number (1-indexed)
- `column` (number): Column number (1-indexed)
- `message` (string): Error message

**Example:**
```lua
cclint.report_error(10, 5, "Use of deprecated function")
```

#### `cclint.report_warning(line, column, message)`

Reports a warning diagnostic.

**Parameters:**
- `line` (number): Line number (1-indexed)
- `column` (number): Column number (1-indexed)
- `message` (string): Warning message

**Example:**
```lua
cclint.report_warning(15, 1, "Line too long")
```

#### `cclint.report_info(line, column, message)`

Reports an informational diagnostic.

**Parameters:**
- `line` (number): Line number (1-indexed)
- `column` (number): Column number (1-indexed)
- `message` (string): Info message

**Example:**
```lua
cclint.report_info(20, 1, "TODO comment found")
```

### Utility Functions

#### `cclint.match_pattern(text, pattern)`

Matches a Lua pattern against text.

**Parameters:**
- `text` (string): Text to search
- `pattern` (string): Lua pattern (not regex)

**Returns:**
- `matched` (boolean): True if pattern matches
- `groups` (table): Table of captured groups (if any)

**Example:**
```lua
local matched, groups = cclint.match_pattern(line, "TODO:%s*(.+)")
if matched then
    print("TODO: " .. groups[1])
end
```

**Note:** This function uses Lua patterns, not regular expressions. See [Lua Pattern Syntax](#lua-pattern-syntax) below.

#### `cclint.get_file_content(path)`

Reads the content of a file.

**Parameters:**
- `path` (string): File path to read

**Returns:**
- `content` (string): File content, or nil on error

**Example:**
```lua
local header_content = cclint.get_file_content("include/header.hpp")
if header_content then
    -- Process header content
end
```

**Security Note:** This function is sandboxed and can only read files within the project directory.

## Rule Structure

A complete rule script should follow this structure:

```lua
-- Metadata (optional but recommended)
rule_description = "Brief description of the rule"
rule_category = "category-name"

-- Main check function (required)
function check_file(file_path)
    -- Access global variables
    local content = file_content
    local lines = file_lines
    local params = rule_params

    -- Iterate through lines
    for line_num, line in ipairs(lines) do
        -- Check condition
        if some_condition(line) then
            cclint.report_warning(line_num, 1, "Problem detected")
        end
    end
end
```

## Lua Pattern Syntax

Lua patterns are similar to regular expressions but simpler. Common patterns:

| Pattern | Meaning |
|---------|---------|
| `.` | Any character |
| `%s` | Whitespace character |
| `%S` | Non-whitespace character |
| `%w` | Alphanumeric character |
| `%W` | Non-alphanumeric character |
| `%d` | Digit |
| `%D` | Non-digit |
| `%a` | Letter |
| `%A` | Non-letter |
| `*` | 0 or more repetitions |
| `+` | 1 or more repetitions |
| `-` | 0 or more repetitions (lazy) |
| `?` | 0 or 1 occurrence |
| `[abc]` | Character class (a, b, or c) |
| `[^abc]` | Negated character class |
| `%f[set]` | Frontier pattern (boundary) |
| `()` | Capture group |

**Examples:**
```lua
-- Match TODO comments
"TODO:%s*(.+)"

-- Match function declarations
"function%s+(%w+)%s*%("

-- Match class names (PascalCase)
"class%s+([A-Z][a-zA-Z0-9]*)"

-- Word boundary
"%f[%w]goto%f[%W]"
```

For full reference: https://www.lua.org/manual/5.1/manual.html#5.4.1

## Complete Example

Here's a complete example rule that detects magic numbers:

```lua
-- Magic Number Detector
rule_description = "Detects magic numbers in code"
rule_category = "readability"

-- Configuration
local ignored_numbers = {0, 1, -1}
if rule_params and rule_params.ignored_numbers then
    -- Parse comma-separated list
    ignored_numbers = {}
    for num in rule_params.ignored_numbers:gmatch("([^,]+)") do
        table.insert(ignored_numbers, tonumber(num))
    end
end

-- Helper function
local function is_ignored(num)
    for _, ignored in ipairs(ignored_numbers) do
        if num == ignored then
            return true
        end
    end
    return false
end

-- Main check function
function check_file(file_path)
    for line_num, line in ipairs(file_lines) do
        -- Skip comments
        if cclint.match_pattern(line, "^%s*//") then
            goto continue
        end

        -- Find numeric literals
        for num_str in line:gmatch("%d+%.?%d*") do
            local num = tonumber(num_str)
            if num and not is_ignored(num) then
                cclint.report_warning(
                    line_num,
                    1,
                    "Magic number detected: " .. num_str ..
                    ". Consider using a named constant."
                )
            end
        end

        ::continue::
    end
end
```

## Configuration Example

To use the above rule in your `.cclint.yaml`:

```yaml
lua_scripts:
  - path: rules/magic_number_detector.lua
    priority: 100
    parameters:
      ignored_numbers: "0,1,-1,2,10,100"
```

## Best Practices

### 1. Use Descriptive Messages

```lua
-- Good
cclint.report_warning(line_num, 1,
    "Function '" .. func_name .. "' exceeds maximum complexity of 10")

-- Bad
cclint.report_warning(line_num, 1, "Too complex")
```

### 2. Check for nil Values

```lua
if rule_params and rule_params.max_length then
    max_length = tonumber(rule_params.max_length) or 80
end
```

### 3. Use Efficient Patterns

```lua
-- Efficient: early exit
if not cclint.match_pattern(line, "class") then
    goto continue
end

-- Less efficient: checking everything
for line_num, line in ipairs(file_lines) do
    -- Many checks
end
```

### 4. Handle Edge Cases

```lua
-- Check for empty lines
if #line == 0 then
    goto continue
end

-- Check for minimum length
if #line < 3 then
    goto continue
end
```

### 5. Document Your Parameters

```lua
-- Configuration (via rule_params):
--   max_length: Maximum line length (default: 80)
--   ignore_comments: Whether to ignore comment lines (default: true)
--   ignore_urls: Whether to ignore lines with URLs (default: true)
```

## Sandbox Limitations

For security, the Lua environment is sandboxed with the following restrictions:

### Disabled Modules/Functions

- `io.*` - File I/O (except `cclint.get_file_content`)
- `os.execute` - System command execution
- `os.exit` - Program termination
- `os.remove`, `os.rename` - File operations
- `loadfile`, `dofile` - Dynamic code loading
- `require` - Module loading (partially)

### Allowed Operations

- String manipulation
- Table operations
- Math functions
- Pattern matching
- `cclint.*` API functions

## Debugging

### Print Statements

Use `print()` to debug your rules. Output appears in cclint's log when run with `-v` flag:

```lua
print("Debug: checking line " .. line_num)
```

Run with:
```bash
cclint -v --config=my-config.yaml g++ test.cpp
```

### Error Handling

Lua errors are caught and reported by cclint:

```lua
function check_file(file_path)
    if not file_content then
        error("No file content available")
    end
    -- ... rest of rule
end
```

## Performance Tips

### 1. Avoid Unnecessary Iteration

```lua
-- Good: early exit
for line_num, line in ipairs(file_lines) do
    if not line:match("keyword") then
        goto continue
    end
    -- Expensive processing
    ::continue::
end
```

### 2. Cache Pattern Matches

```lua
-- Good: compile pattern once
local pattern = "class%s+(%w+)"
for line_num, line in ipairs(file_lines) do
    local matched, groups = cclint.match_pattern(line, pattern)
end
```

### 3. Use Local Variables

```lua
-- Good: local variables are faster
local function check_line(line, line_num)
    local max_len = 80
    -- ...
end
```

## FAQ

### Q: Can I access other files from my rule?

A: Yes, use `cclint.get_file_content(path)`, but only files within the project directory are accessible.

### Q: Can I use external Lua libraries?

A: No, `require()` and module loading are disabled for security. All functionality must be self-contained.

### Q: How do I test my rule?

A: Create a test C++ file with known violations and run cclint with your rule:

```bash
cclint --config=test-config.yaml g++ test.cpp
```

### Q: Can I disable a rule for specific lines?

A: This feature is planned for future releases. Currently, rules apply to all matching code.

### Q: What's the difference between patterns and regex?

A: Lua patterns are simpler and faster than full regular expressions. They lack some features like alternation `|` and lookahead/lookbehind.

## See Also

- [Lua 5.1 Reference Manual](https://www.lua.org/manual/5.1/)
- [Rule Examples](../examples/rules/)
- [Configuration Guide](usage.md)
- [Creating Custom Rules Guide](../examples/rules/README.md)
