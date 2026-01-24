# cclint Plugin Development Guide

This guide explains how to create custom linting rules as plugins for cclint.

## Overview

cclint supports two ways to extend functionality with custom rules:

1. **Lua Scripts**: Lightweight, easy to write, ideal for most use cases
2. **C++ Plugins**: Compiled shared libraries, ideal for complex logic or performance-critical rules

This guide focuses on C++ plugins.

## Plugin API

Plugins are shared libraries (.so on Linux, .dylib on macOS, .dll on Windows) that implement the cclint Plugin API.

### Required Functions

Your plugin must export the following C functions:

```cpp
extern "C" {
    const char* cclint_plugin_get_name();
    const char* cclint_plugin_get_description();
    const char* cclint_plugin_get_category();
    void* cclint_plugin_create_rule();
    void cclint_plugin_destroy_rule(void* rule);
}
```

### RuleBase Interface

The rule object created by `cclint_plugin_create_rule()` must inherit from `cclint::rules::RuleBase`:

```cpp
class RuleBase {
public:
    virtual void check_file(
        const std::string& file_path,
        const std::string& file_content,
        cclint::diagnostic::DiagnosticEngine& diag_engine) = 0;

    virtual void check_ast(
        const std::string& file_path,
        std::shared_ptr<cclint::parser::TranslationUnitNode> ast,
        cclint::diagnostic::DiagnosticEngine& diag_engine) = 0;
};
```

## Example Plugin

Here's a complete example plugin that detects TODO comments:

```cpp
#include "cclint/plugin_api.h"
#include "rules/rule_base.hpp"
#include "diagnostic/diagnostic.hpp"
#include "parser/ast.hpp"

namespace {

class TodoDetectorRule : public cclint::rules::RuleBase {
public:
    TodoDetectorRule() {
        name_ = "todo-detector";
        description_ = "Detects TODO comments in code";
        category_ = "documentation";
    }

    void check_file(const std::string& file_path,
                   const std::string& file_content,
                   cclint::diagnostic::DiagnosticEngine& diag_engine) override {
        size_t line_num = 1;
        size_t pos = 0;

        while (pos < file_content.size()) {
            size_t line_end = file_content.find('\n', pos);
            if (line_end == std::string::npos) {
                line_end = file_content.size();
            }

            std::string line = file_content.substr(pos, line_end - pos);

            // Check for TODO comments
            size_t todo_pos = line.find("TODO");
            if (todo_pos != std::string::npos) {
                cclint::diagnostic::Diagnostic diag;
                diag.severity = cclint::diagnostic::Severity::Info;
                diag.rule_name = name_;
                diag.message = "TODO comment found";
                diag.location.filename = file_path;
                diag.location.line = line_num;
                diag.location.column = todo_pos;
                diag_engine.add_diagnostic(diag);
            }

            pos = line_end + 1;
            line_num++;
        }
    }

    void check_ast(const std::string& file_path,
                   std::shared_ptr<cclint::parser::TranslationUnitNode> ast,
                   cclint::diagnostic::DiagnosticEngine& diag_engine) override {
        // Optional: AST-based checks
        (void)file_path;
        (void)ast;
        (void)diag_engine;
    }
};

} // anonymous namespace

// Plugin API implementation
extern "C" {

const char* cclint_plugin_get_name() {
    return "todo-detector";
}

const char* cclint_plugin_get_description() {
    return "Detects TODO comments in code";
}

const char* cclint_plugin_get_category() {
    return "documentation";
}

void* cclint_plugin_create_rule() {
    return new TodoDetectorRule();
}

void cclint_plugin_destroy_rule(void* rule) {
    delete static_cast<TodoDetectorRule*>(rule);
}

} // extern "C"
```

## Building Plugins

### CMake Configuration

Create a `CMakeLists.txt` for your plugin:

```cmake
cmake_minimum_required(VERSION 3.14)

add_library(my_plugin SHARED
    my_plugin.cpp
)

target_include_directories(my_plugin
    PRIVATE
    ${CCLINT_SOURCE_DIR}/src
    ${CCLINT_SOURCE_DIR}/include
)

set_target_properties(my_plugin PROPERTIES
    PREFIX ""  # Don't add 'lib' prefix
    OUTPUT_NAME "my_plugin"
)

install(TARGETS my_plugin
    LIBRARY DESTINATION lib/cclint/plugins
    RUNTIME DESTINATION lib/cclint/plugins
)
```

### Build Commands

```bash
mkdir build && cd build
cmake ..
make
```

This will create:
- Linux: `my_plugin.so`
- macOS: `my_plugin.dylib`
- Windows: `my_plugin.dll`

## Using Plugins

### Loading Plugins

Plugins can be loaded in two ways:

1. **Configuration File** (`.cclint.yaml`):

```yaml
plugins:
  - path: /path/to/my_plugin.so
    enabled: true
  - path: plugins/another_plugin.dylib
    enabled: true
```

2. **Plugin Directory**:

Place plugins in `~/.cclint/plugins/` or specify with `--plugin-dir`:

```bash
cclint --plugin-dir=/path/to/plugins g++ test.cpp
```

## Best Practices

### 1. Implement Both Check Methods

Even if you only need one, implement both `check_file` and `check_ast`:

```cpp
void check_file(...) override {
    // Your text-based check
}

void check_ast(...) override {
    // No-op if not needed
    (void)file_path;
    (void)ast;
    (void)diag_engine;
}
```

### 2. Use Descriptive Names

```cpp
const char* cclint_plugin_get_name() {
    return "my-company-naming-convention";  // Good
    // return "rule1";  // Bad
}
```

### 3. Provide Helpful Messages

```cpp
diag.message = "Function name '" + func_name +
               "' violates naming convention (expected camelCase)";
// Not: "Bad name"
```

### 4. Handle Edge Cases

```cpp
void check_file(...) override {
    if (file_content.empty()) {
        return;  // Nothing to check
    }
    // ... rest of implementation
}
```

### 5. Set Appropriate Severity

```cpp
// Critical issues: Error
diag.severity = cclint::diagnostic::Severity::Error;

// Style violations: Warning
diag.severity = cclint::diagnostic::Severity::Warning;

// Suggestions: Info
diag.severity = cclint::diagnostic::Severity::Info;
```

## Advanced Features

### AST-Based Checks

Access the parsed AST for structural analysis:

```cpp
void check_ast(const std::string& file_path,
               std::shared_ptr<cclint::parser::TranslationUnitNode> ast,
               cclint::diagnostic::DiagnosticEngine& diag_engine) override {
    // Traverse AST
    std::function<void(std::shared_ptr<cclint::parser::ASTNode>)> traverse;
    traverse = [&](std::shared_ptr<cclint::parser::ASTNode> node) {
        if (!node) return;

        if (node->type == cclint::parser::ASTNodeType::Function) {
            auto func = std::dynamic_pointer_cast<cclint::parser::FunctionNode>(node);
            if (func) {
                // Check function
                check_function(func, diag_engine);
            }
        }

        for (const auto& child : node->children) {
            traverse(child);
        }
    };

    traverse(ast);
}
```

### Fix-It Hints

Provide automatic fix suggestions:

```cpp
cclint::diagnostic::Diagnostic diag;
diag.severity = cclint::diagnostic::Severity::Warning;
diag.rule_name = name_;
diag.message = "Use 'nullptr' instead of 'NULL'";

// Add fix-it hint
cclint::diagnostic::FixItHint hint;
hint.range.begin.filename = file_path;
hint.range.begin.line = line_num;
hint.range.begin.column = col;
hint.range.end.filename = file_path;
hint.range.end.line = line_num;
hint.range.end.column = col + 4;  // Length of "NULL"
hint.replacement_text = "nullptr";

diag.fix_hints.push_back(hint);
diag_engine.add_diagnostic(diag);
```

## Testing Plugins

### Unit Testing

Create test files with known violations:

```cpp
// test_file.cpp
void test() {
    // TODO: implement this
    int x = NULL;  // Should trigger nullptr rule
}
```

Run cclint:

```bash
cclint --config=test-config.yaml g++ test_file.cpp
```

### Integration Testing

Test plugin loading:

```bash
# Test plugin loads
cclint --list-rules

# Test plugin executes
cclint --config=config.yaml g++ file.cpp
```

## Debugging Plugins

### Enable Verbose Output

```bash
cclint -v --config=config.yaml g++ test.cpp
```

### Use Debug Builds

Build with debug symbols:

```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

### Print Debug Info

```cpp
void check_file(...) override {
    std::cerr << "DEBUG: Checking file: " << file_path << std::endl;
    // ... rest of implementation
}
```

## Distribution

### Package Structure

```
my_plugin/
├── CMakeLists.txt
├── README.md
├── src/
│   └── my_plugin.cpp
├── include/
│   └── my_plugin.hpp
└── tests/
    ├── test_file.cpp
    └── test_config.yaml
```

### Installation

```bash
# Install to system
cmake --install build --prefix /usr/local

# Install to user directory
cmake --install build --prefix ~/.cclint
```

## See Also

- [Plugin API Header](../include/cclint/plugin_api.h)
- [Example Plugin](../examples/plugins/example_plugin.cpp)
- [RuleBase Documentation](../src/rules/rule_base.hpp)
- [Diagnostic API](../src/diagnostic/diagnostic.hpp)
