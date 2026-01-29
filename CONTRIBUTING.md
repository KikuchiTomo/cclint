# Contributing to cclint

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

## Coding Style

### Naming
```cpp
class ClassName { };              // PascalCase
void function_name();             // snake_case
int variable_name = 0;            // snake_case
const int CONSTANT_NAME = 100;    // UPPER_CASE

class Config {
private:
    std::string member_;          // trailing underscore
};

namespace namespace_name { }      // snake_case
```

### Files
- Header files: `.hpp`
- Source files: `.cpp`
- Always use `#pragma once`

### Format
```bash
clang-format -i src/**/*.{cpp,hpp}
```

## Commits

Use [Conventional Commits](https://www.conventionalcommits.org/):

```
<type>: <subject>

<body>

Co-Authored-By: Claude Sonnet 4.5 <noreply@anthropic.com>
```

**Types:** feat, fix, docs, style, refactor, perf, test, chore, ci

**Examples:**
```
feat: add constructor detection to AST

Added ConstructorNode to AST with metadata:
- explicit, default, delete specifiers
- parameter types
- access specifiers

Closes #42
```

## Lua Rules

Create `.lua` file:

```lua
rule_description = "Rule description"
rule_category = "custom"

function check_file()
    for line_num, line in ipairs(file_lines) do
        if line:match("pattern") then
            cclint.report_warning(line_num, 1, "Message")
        end
    end
end

function check_ast()
    local classes = cclint.get_classes()
    for _, cls in ipairs(classes) do
        -- Check AST nodes
    end
end
```

Place in `samples/rules/` or configure in `.cclint.yaml`.

## Pull Requests

- Build without errors
- Follow coding style
- Write clear commit messages
- Update documentation if needed
