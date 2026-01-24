# cclint Usage Guide

This guide explains how to use cclint to lint your C++ code.

## Basic Usage

cclint works as a wrapper around your existing compiler commands:

```bash
cclint <compiler-command>
```

### Simple Examples

```bash
# Lint a single file
cclint g++ -std=c++17 main.cpp

# Lint multiple files
cclint g++ -std=c++17 src/*.cpp

# With optimization flags
cclint g++ -std=c++17 -O2 main.cpp

# With include paths
cclint g++ -std=c++17 -I./include main.cpp
```

## Command-Line Options

### General Options

```bash
cclint [OPTIONS] <compiler-command>

Options:
  --help              Show help message and exit
  --version           Show version information and exit
  --config=FILE       Specify configuration file path
  --format=FORMAT     Output format: text, json, xml (default: text)
  -v, --verbose       Verbose output (show INFO messages)
  -q, --quiet         Quiet mode (show ERROR messages only)
```

### Examples

```bash
# Show help
cclint --help

# Show version
cclint --version

# Use custom config file
cclint --config=.cclint.yaml g++ main.cpp

# JSON output for CI/CD
cclint --format=json g++ main.cpp

# XML output for IDE integration
cclint --format=xml g++ main.cpp

# Verbose output
cclint -v g++ -std=c++17 main.cpp

# Quiet mode (errors only)
cclint -q g++ -std=c++17 main.cpp
```

## Configuration

### Configuration File

cclint searches for configuration files in this order:

1. `.cclint.yaml` (current directory)
2. `.cclint.yml` (current directory)
3. `cclint.yaml` (current directory)
4. `cclint.yml` (current directory)
5. Parent directories (up to project root)
6. `~/.cclint/config.yaml` (home directory)

### Configuration File Format

Create a `cclint.yaml` file in your project root:

```yaml
version: 1.0

# C++ standard (auto-detect from compiler flags if not specified)
cpp_standard: cpp17

# File patterns to include
include_patterns:
  - "src/**/*.cpp"
  - "src/**/*.hpp"
  - "include/**/*.h"

# File patterns to exclude
exclude_patterns:
  - "third_party/**"
  - "build/**"
  - "*.pb.h"
  - "*.pb.cc"

# Lua rule scripts
lua_scripts:
  # Use standard rules from cclint installation
  - path: "${CCLINT_HOME}/scripts/rules/naming/class-name-camelcase.lua"
    priority: 100

  - path: "${CCLINT_HOME}/scripts/rules/modernize/use-nullptr.lua"
    priority: 90

  # Use custom rules from your project
  - path: ".cclint/custom-rules.lua"
    priority: 80

# Output settings
output_format: text
max_errors: 0  # 0 = unlimited

# Show compiler output
show_compiler_output: true

# Performance settings
num_threads: 0  # 0 = auto-detect
enable_cache: true
cache_directory: ".cclint_cache"

# Rule execution settings
parallel_rules: true
fail_fast: false  # Stop on first error
```

### Configuration Examples

#### Minimal Configuration

```yaml
version: 1.0
lua_scripts:
  - path: "${CCLINT_HOME}/scripts/rules/naming/class-name-camelcase.lua"
```

#### Strict Configuration

```yaml
version: 1.0
cpp_standard: cpp17

lua_scripts:
  # Naming conventions
  - path: "${CCLINT_HOME}/scripts/rules/naming/class-name-camelcase.lua"
  - path: "${CCLINT_HOME}/scripts/rules/naming/function-name-lowercase.lua"
  - path: "${CCLINT_HOME}/scripts/rules/naming/constant-name-uppercase.lua"

  # Modern C++ features
  - path: "${CCLINT_HOME}/scripts/rules/modernize/use-nullptr.lua"
  - path: "${CCLINT_HOME}/scripts/rules/modernize/use-override.lua"
  - path: "${CCLINT_HOME}/scripts/rules/modernize/use-auto.lua"

  # Performance
  - path: "${CCLINT_HOME}/scripts/rules/performance/pass-by-const-reference.lua"
  - path: "${CCLINT_HOME}/scripts/rules/performance/reserve-vector.lua"

  # Security
  - path: "${CCLINT_HOME}/scripts/rules/security/no-unsafe-functions.lua"
  - path: "${CCLINT_HOME}/scripts/rules/security/check-array-bounds.lua"

max_errors: 100
fail_fast: false
```

## Output Formats

### Text Output (Default)

Human-readable output with colors:

```
src/main.cpp:42:5: warning: Function name should be lowercase [function-name-lowercase]
    void DoSomething() {
    ^

src/utils.hpp:15:1: error: Missing header guard [header-guard]

Summary: 1 error, 1 warning
```

### JSON Output

Machine-readable JSON for CI/CD integration:

```bash
cclint --format=json g++ main.cpp > results.json
```

```json
{
  "diagnostics": [
    {
      "severity": "warning",
      "rule": "function-name-lowercase",
      "message": "Function name should be lowercase",
      "location": {
        "file": "src/main.cpp",
        "line": 42,
        "column": 5
      }
    }
  ],
  "summary": {
    "errors": 1,
    "warnings": 1,
    "total": 2
  }
}
```

### XML Output

Checkstyle-compatible XML for IDE integration:

```bash
cclint --format=xml g++ main.cpp > checkstyle-result.xml
```

```xml
<?xml version="1.0" encoding="UTF-8"?>
<checkstyle version="1.0">
  <file name="src/main.cpp">
    <error line="42" column="5" severity="warning"
           message="Function name should be lowercase"
           source="function-name-lowercase"/>
  </file>
</checkstyle>
```

## Integration Examples

### Makefile Integration

```makefile
# Add cclint target to your Makefile
lint:
	cclint g++ -std=c++17 $(SOURCES)

# Or integrate into build
all: lint
	g++ -std=c++17 $(SOURCES) -o $(TARGET)
```

### CMake Integration

```cmake
# Add custom target for linting
add_custom_target(lint
    COMMAND cclint ${CMAKE_CXX_COMPILER} ${CMAKE_CXX_FLAGS} ${SOURCES}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running cclint..."
)
```

### CI/CD Integration

#### GitHub Actions

```yaml
name: Lint

on: [push, pull_request]

jobs:
  lint:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2

      - name: Install cclint
        run: |
          # Installation steps

      - name: Run cclint
        run: |
          cclint --format=json g++ -std=c++17 src/*.cpp > lint-results.json

      - name: Upload results
        uses: actions/upload-artifact@v2
        with:
          name: lint-results
          path: lint-results.json
```

#### GitLab CI

```yaml
lint:
  stage: test
  script:
    - cclint --format=json g++ -std=c++17 src/*.cpp > lint-results.json
  artifacts:
    reports:
      codequality: lint-results.json
```

## Exit Codes

cclint uses the following exit codes:

- `0`: Success (no errors found)
- `1`: Lint errors found
- `2`: Runtime error (invalid options, config error, etc.)

Example usage in scripts:

```bash
if cclint g++ main.cpp; then
    echo "No lint errors found"
else
    echo "Lint errors detected"
    exit 1
fi
```

## Common Workflows

### Pre-commit Hook

Create `.git/hooks/pre-commit`:

```bash
#!/bin/bash
# Run cclint on staged C++ files

STAGED_FILES=$(git diff --cached --name-only --diff-filter=ACM | grep -E '\.(cpp|hpp|cc|h)$')

if [ -n "$STAGED_FILES" ]; then
    echo "Running cclint on staged files..."
    cclint g++ -std=c++17 $STAGED_FILES

    if [ $? -ne 0 ]; then
        echo "cclint found issues. Please fix them before committing."
        exit 1
    fi
fi
```

Make it executable:

```bash
chmod +x .git/hooks/pre-commit
```

### Development Workflow

```bash
# 1. Make code changes
vim src/mycode.cpp

# 2. Run linter
cclint -v g++ -std=c++17 src/mycode.cpp

# 3. Fix issues
vim src/mycode.cpp

# 4. Re-run linter
cclint g++ -std=c++17 src/mycode.cpp

# 5. Commit when clean
git add src/mycode.cpp
git commit -m "Add new feature"
```

## Lua Rule Scripts

cclint includes 100 standard Lua rule scripts organized by category:

### Available Rule Categories

- **Naming** (9 rules): Class, function, constant, enum naming conventions
- **Style** (8 rules): Bracing, indentation, line length, spacing
- **Structure** (4 rules): File organization, header guards, includes
- **Spacing** (5 rules): Empty lines, whitespace, operators
- **Documentation** (4 rules): Comments, headers, TODO format
- **Modernize** (18 rules): C++11/14/17/20 features
- **Performance** (15 rules): Efficiency, unnecessary copies
- **Readability** (19 rules): Complexity, magic numbers, expressions
- **Security** (11 rules): Unsafe functions, buffer overflows

### Rule Locations

Standard rules are installed to:
- Linux: `/usr/local/share/cclint/scripts/rules/`
- macOS: `/usr/local/share/cclint/scripts/rules/`

Use `${CCLINT_HOME}` in your config to reference them:

```yaml
lua_scripts:
  - path: "${CCLINT_HOME}/scripts/rules/naming/class-name-camelcase.lua"
```

See [scripts/README.md](../scripts/README.md) for the complete list of available rules.

## Troubleshooting

### No Compiler Command Specified

```bash
$ cclint
Error: No compiler command specified
```

**Solution**: Always provide a compiler command:
```bash
cclint g++ -std=c++17 main.cpp
```

### Config File Not Found

cclint will use default configuration if no config file is found. To verify which config is being used:

```bash
cclint -v g++ main.cpp
# Look for: "Loaded config from: ..." or "Using default configuration"
```

### Compiler Not Detected

If cclint can't detect your compiler:

```bash
[WARN] Could not detect compiler type
```

This doesn't prevent cclint from working, but may affect some features. Make sure your compiler is in PATH.

## Current Limitations

**Note**: This is an alpha release (v0.1.0-alpha). The following features are not yet implemented:

- ⏳ C++ AST parsing (Milestone 2)
- ⏳ Lua rule execution (Milestone 3)
- ⏳ YAML configuration parsing (Milestone 2)

The current version:
- ✅ Wraps compiler commands
- ✅ Detects compilers
- ✅ Extracts source files and flags
- ✅ Supports multiple output formats
- ✅ Has 100 Lua rule scripts ready for integration

## Next Steps

- Read [Lua API Reference](lua_api.md) (coming soon)
- Read [Creating Custom Rules](creating_lua_rules.md) (coming soon)
- Check [TODO.md](TODO.md) for development status
- Report issues on [GitHub](https://github.com/yourusername/cclint/issues)
