# Contributing to cclint

Thank you for your interest in contributing to cclint! This document provides guidelines and instructions for contributing.

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [How Can I Contribute?](#how-can-i-contribute)
- [Development Setup](#development-setup)
- [Coding Guidelines](#coding-guidelines)
- [Commit Guidelines](#commit-guidelines)
- [Pull Request Process](#pull-request-process)
- [Testing](#testing)
- [Documentation](#documentation)

## Code of Conduct

This project adheres to a Code of Conduct (see [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md)). By participating, you are expected to uphold this code.

## How Can I Contribute?

### Reporting Bugs

Before creating bug reports, please check the [existing issues](https://github.com/yourusername/cclint/issues) to avoid duplicates.

When creating a bug report, include:
- **Clear title**: Describe the issue succinctly
- **Steps to reproduce**: Provide a minimal reproduction case
- **Expected behavior**: What you expected to happen
- **Actual behavior**: What actually happened
- **Environment**:
  - cclint version (`cclint --version`)
  - Operating system and version
  - Compiler and version
  - Any relevant configuration

**Example**:
```
Title: cclint crashes with --format=xml on Ubuntu 22.04

Steps to reproduce:
1. Create test.cpp with: int main() { return 0; }
2. Run: cclint --format=xml g++ test.cpp
3. Observe crash

Expected: XML output
Actual: Segmentation fault

Environment:
- cclint 0.1.0-alpha
- Ubuntu 22.04
- GCC 11.3.0
```

### Suggesting Features

Feature suggestions are welcome! Before suggesting:
1. Check if it's already [planned](docs/milestones.md)
2. Search [existing feature requests](https://github.com/yourusername/cclint/issues?q=is%3Aissue+label%3Aenhancement)

When suggesting a feature:
- **Use case**: Why is this feature needed?
- **Proposed solution**: How should it work?
- **Alternatives**: What alternatives did you consider?
- **Examples**: Provide concrete examples

### Contributing Code

We welcome code contributions! You can:
- Fix bugs
- Implement planned features
- Add new Lua rules
- Improve documentation
- Write tests
- Optimize performance

## Development Setup

### Prerequisites

- **Build tools**:
  - CMake 3.16+
  - C++17 compiler (GCC 7+, Clang 10+)
- **Optional** (for full functionality):
  - LLVM/Clang 14.0+ (Milestone 2)
  - yaml-cpp 0.7.0+ (Milestone 2)
  - LuaJIT 2.1+ (Milestone 3)
  - Google Test (for tests)

### Fork and Clone

```bash
# Fork the repository on GitHub, then:
git clone https://github.com/YOUR_USERNAME/cclint.git
cd cclint

# Add upstream remote
git remote add upstream https://github.com/yourusername/cclint.git
```

### Build

```bash
# Quick build
./build.sh

# Or manually
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)
```

### Run Tests

```bash
# When tests are implemented
cd build
ctest --verbose
```

### Development Workflow

1. Create a feature branch:
```bash
git checkout -b feature/my-feature
```

2. Make changes and test:
```bash
# Edit code
vim src/cli/argument_parser.cpp

# Build
./build.sh

# Test
./build/src/cclint --help
```

3. Commit changes:
```bash
git add src/cli/argument_parser.cpp
git commit -m "feat: add --exclude option to CLI parser"
```

4. Push and create PR:
```bash
git push origin feature/my-feature
# Create Pull Request on GitHub
```

## Coding Guidelines

### C++ Style

We follow a modified Google C++ Style Guide. Key points:

#### Naming

```cpp
// Classes: PascalCase
class ArgumentParser { };

// Functions: snake_case
void parse_arguments();

// Variables: snake_case
int file_count = 0;

// Constants: UPPER_CASE
const int MAX_ERRORS = 100;

// Member variables: trailing underscore
class Config {
private:
    std::string version_;
    int max_errors_;
};

// Namespaces: snake_case
namespace cclint {
namespace config {
}
}
```

#### Files

- Header files: `.hpp`
- Source files: `.cpp`
- Always use `#pragma once` (not include guards)
- One class per file (generally)

#### Modern C++

```cpp
// Use smart pointers
auto rule = std::make_unique<Rule>();  // ✅
Rule* rule = new Rule();  // ❌

// Use auto when type is obvious
auto config = load_config();  // ✅

// Use const references for parameters
void process(const std::vector<Diagnostic>& diags);  // ✅
void process(std::vector<Diagnostic> diags);  // ❌

// Use range-based for loops
for (const auto& item : vec) { }  // ✅
for (auto it = vec.begin(); it != vec.end(); ++it) { }  // ❌

// Use nullptr
int* ptr = nullptr;  // ✅
int* ptr = NULL;  // ❌
```

### Code Formatting

We use clang-format with the included `.clang-format` file:

```bash
# Format all files
find src -name '*.cpp' -o -name '*.hpp' | xargs clang-format -i

# Format specific file
clang-format -i src/cli/argument_parser.cpp
```

### Static Analysis

Run clang-tidy before committing:

```bash
clang-tidy src/**/*.cpp -- -std=c++17
```

### Documentation

- Add comments for non-obvious code
- Document public APIs with Doxygen-style comments:

```cpp
/// Parses command-line arguments.
///
/// @param argc Number of arguments
/// @param argv Array of argument strings
/// @return Parsed arguments structure
/// @throws std::invalid_argument if arguments are invalid
ParsedArguments parse(int argc, char** argv);
```

## Commit Guidelines

We use [Conventional Commits](https://www.conventionalcommits.org/):

### Format

```
<type>: <subject>

<body>

<footer>
```

### Types

- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `style`: Code style changes (formatting, etc.)
- `refactor`: Code refactoring
- `perf`: Performance improvements
- `test`: Adding or updating tests
- `chore`: Build process, dependencies, etc.
- `ci`: CI/CD changes

### Examples

```
feat: add --exclude option to CLI parser

Added support for excluding files from analysis using
glob patterns via the --exclude option.

Closes #42
```

```
fix: correct config file search order

Changed search order to match documentation:
1. .cclint.yaml (current dir)
2. Parent directories
3. ~/.cclint/config.yaml

Fixes #58
```

```
docs: add FAQ for common issues

Added comprehensive FAQ covering:
- Installation
- Configuration
- Troubleshooting
- Custom rules
```

### Commit Best Practices

- Keep commits atomic (one logical change per commit)
- Write clear, descriptive messages
- Reference issues/PRs when relevant
- Use imperative mood ("add feature" not "added feature")

## Pull Request Process

### Before Submitting

- [ ] Code builds without errors
- [ ] All tests pass (when applicable)
- [ ] Code follows style guidelines
- [ ] Documentation is updated
- [ ] CHANGELOG.md is updated (for significant changes)
- [ ] Commits follow commit guidelines

### Submitting

1. **Title**: Clear and descriptive
   - Good: "Add support for custom output formatters"
   - Bad: "Fix stuff"

2. **Description**: Explain what and why
   ```markdown
   ## Summary
   Adds support for custom output formatters allowing users to
   define their own output formats via Lua scripts.

   ## Changes
   - Added FormatterRegistry class
   - Implemented Lua formatter API
   - Updated documentation

   ## Testing
   - Added unit tests for FormatterRegistry
   - Tested with sample custom formatter
   - All existing tests pass

   Closes #123
   ```

3. **Labels**: Add appropriate labels (bug, enhancement, documentation, etc.)

### Review Process

1. Maintainers will review your PR
2. Address review comments
3. Push updates to your branch
4. PR will be merged when approved

### After Merge

1. Delete your feature branch
2. Pull latest changes:
```bash
git checkout main
git pull upstream main
```

## Testing

### Writing Tests

Use Google Test framework:

```cpp
#include <gtest/gtest.h>
#include "cli/argument_parser.hpp"

TEST(ArgumentParserTest, ParseHelp) {
    cli::ArgumentParser parser;
    const char* argv[] = {"cclint", "--help"};
    auto args = parser.parse(2, const_cast<char**>(argv));

    EXPECT_TRUE(args.show_help);
}
```

### Test Categories

- **Unit tests**: Test individual components (`tests/unit/`)
- **Integration tests**: Test complete workflows (`tests/integration/`)
- **Sample code**: Test files for validation (`tests/samples/`)

### Running Tests

```bash
# All tests
cd build
ctest

# Specific test
./tests/unit/test_argument_parser

# With Google Test filters
./tests/cclint_tests --gtest_filter=ArgumentParser*
```

## Documentation

### What to Document

- **Public APIs**: All public functions and classes
- **Complex logic**: Non-obvious algorithms
- **Configuration**: New config options
- **Usage examples**: How to use new features
- **Changelog**: Significant changes

### Where to Document

- **Code**: Inline comments and Doxygen
- **README.md**: Project overview
- **docs/**: Guides and references
  - `build.md`: Build instructions
  - `usage.md`: Usage guide
  - `troubleshooting.md`: Common issues
  - `FAQ.md`: Frequently asked questions
- **CHANGELOG.md**: Version history
- **examples/**: Usage examples

### Documentation Style

- Clear and concise
- Use examples
- Keep up to date with code
- Proofread before committing

## Lua Rule Development

### Writing Rules

Create a Lua file in `scripts/rules/<category>/`:

```lua
local rule = {
    name = "my-rule",
    description = "Checks for something",
    severity = "warning"
}

function rule:init(params)
    self.threshold = params.threshold or 10
end

function rule:visit_function_decl(func_decl)
    if func_decl:get_name():len() > self.threshold then
        self:report_diagnostic({
            severity = self.severity,
            message = "Function name too long",
            location = func_decl:get_location()
        })
    end
end

return rule
```

### Testing Rules

1. Create test file in `tests/samples/`
2. Run cclint with your rule
3. Verify diagnostic output

### Submitting Rules

1. Add rule to appropriate category
2. Update `scripts/README.md`
3. Add example to `.cclint.example.yaml`
4. Create test case
5. Submit PR

## Questions?

- Check [docs/FAQ.md](docs/FAQ.md)
- Ask on [GitHub Discussions](https://github.com/yourusername/cclint/discussions)
- Open an [issue](https://github.com/yourusername/cclint/issues)

---

**Thank you for contributing to cclint!** 🎉
