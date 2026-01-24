# cclint Tests

This directory contains the test suite for cclint.

## Directory Structure

```
tests/
├── unit/           # Unit tests for individual components
├── integration/    # Integration tests for end-to-end functionality
├── samples/        # Sample C++ code for testing
├── CMakeLists.txt  # Test build configuration
└── README.md       # This file
```

## Test Categories

### Unit Tests (`unit/`)

Unit tests verify the functionality of individual components in isolation.

**Planned tests**:
- `test_argument_parser.cpp` - CLI argument parsing
- `test_config_loader.cpp` - Configuration loading
- `test_compiler_wrapper.cpp` - Compiler command execution
- `test_compiler_detector.cpp` - Compiler detection
- `test_diagnostic.cpp` - Diagnostic message handling
- `test_formatter.cpp` - Output formatting (text/JSON/XML)
- `test_file_utils.cpp` - File operations
- `test_string_utils.cpp` - String utilities
- `test_logger.cpp` - Logger functionality

### Integration Tests (`integration/`)

Integration tests verify the complete workflow of cclint.

**Planned tests**:
- `test_basic_execution.cpp` - Basic cclint execution
- `test_config_integration.cpp` - Configuration file integration
- `test_compiler_integration.cpp` - Compiler wrapper integration
- `test_output_formats.cpp` - Output format generation
- `test_error_handling.cpp` - Error handling scenarios

### Sample Code (`samples/`)

Sample C++ files used for testing cclint functionality.

**Categories**:
- `valid/` - Valid C++ code that should compile
- `invalid/` - Invalid C++ code with syntax errors
- `style_issues/` - Code with style violations
- `security_issues/` - Code with security vulnerabilities
- `performance_issues/` - Code with performance problems

## Running Tests

### Prerequisites

```bash
# Install Google Test
# Ubuntu
sudo apt install libgtest-dev

# macOS
brew install googletest
```

### Build Tests

```bash
# Configure with tests enabled
mkdir build && cd build
cmake -DBUILD_TESTS=ON ..

# Build all tests
make tests

# Or use the build script
cd ..
./build.sh --tests
```

### Run All Tests

```bash
cd build
ctest

# Or run with verbose output
ctest --verbose

# Or run directly
./tests/cclint_tests
```

### Run Specific Test

```bash
# Run only unit tests
./tests/unit/test_argument_parser

# Run only integration tests
./tests/integration/test_basic_execution

# Run with Google Test filters
./tests/cclint_tests --gtest_filter=ArgumentParser*
```

## Writing Tests

### Unit Test Example

```cpp
#include <gtest/gtest.h>
#include "cli/argument_parser.hpp"

TEST(ArgumentParserTest, ParseBasicOptions) {
    cli::ArgumentParser parser;
    const char* argv[] = {"cclint", "--help"};
    auto args = parser.parse(2, const_cast<char**>(argv));

    EXPECT_TRUE(args.show_help);
    EXPECT_FALSE(args.show_version);
}

TEST(ArgumentParserTest, ParseCompilerCommand) {
    cli::ArgumentParser parser;
    const char* argv[] = {"cclint", "g++", "main.cpp"};
    auto args = parser.parse(3, const_cast<char**>(argv));

    ASSERT_EQ(args.compiler_command.size(), 2);
    EXPECT_EQ(args.compiler_command[0], "g++");
    EXPECT_EQ(args.compiler_command[1], "main.cpp");
}
```

### Integration Test Example

```cpp
#include <gtest/gtest.h>
#include <fstream>
#include <cstdlib>

TEST(IntegrationTest, BasicExecution) {
    // Create a simple test file
    std::ofstream test_file("test_temp.cpp");
    test_file << "#include <iostream>\n";
    test_file << "int main() { return 0; }\n";
    test_file.close();

    // Run cclint
    int result = std::system("./cclint g++ test_temp.cpp > /dev/null 2>&1");

    // Check exit code
    EXPECT_EQ(WEXITSTATUS(result), 0);

    // Clean up
    std::remove("test_temp.cpp");
}
```

## Test Guidelines

### Best Practices

1. **Test One Thing**: Each test should verify one specific behavior
2. **Clear Names**: Use descriptive test names (e.g., `ParseBasicOptions`, not `Test1`)
3. **Arrange-Act-Assert**: Structure tests in three sections:
   - Arrange: Set up test data
   - Act: Execute the code under test
   - Assert: Verify the results
4. **Independent Tests**: Tests should not depend on each other
5. **Clean Up**: Always clean up temporary files and resources

### Naming Conventions

- Test files: `test_<component>.cpp`
- Test suites: `<ComponentName>Test`
- Test cases: `<MethodName>_<Scenario>`

Examples:
```cpp
TEST(ArgumentParserTest, Parse_WithHelpFlag_SetsShowHelp)
TEST(ConfigLoaderTest, Load_WithInvalidFile_ThrowsException)
TEST(CompilerWrapperTest, Execute_WithValidCommand_ReturnsZeroExitCode)
```

### Assertions

Use appropriate Google Test assertions:
- `EXPECT_TRUE(condition)` - Non-fatal assertion
- `ASSERT_TRUE(condition)` - Fatal assertion (stops test on failure)
- `EXPECT_EQ(expected, actual)` - Check equality
- `EXPECT_THROW(statement, exception)` - Check exception
- `EXPECT_DEATH(statement, regex)` - Check program termination

## Current Status

**Note**: This is an alpha release. The testing framework is being set up.

### Implemented:
- ✅ Test directory structure
- ✅ Test documentation (this file)
- ✅ Sample code organization

### Pending:
- ⏳ Google Test integration (Milestone 1)
- ⏳ Unit tests for all modules (Milestone 1-2)
- ⏳ Integration tests (Milestone 2)
- ⏳ Test coverage reporting (Milestone 4)
- ⏳ Automated test execution in CI/CD (Milestone 5)

## Coverage Goals

- **Unit Tests**: Aim for 80%+ code coverage
- **Integration Tests**: Cover all main use cases
- **Regression Tests**: Add tests for all reported bugs

## Contributing

When adding new features:
1. Write tests first (TDD approach)
2. Ensure all tests pass before committing
3. Add integration tests for user-facing features
4. Update this README if adding new test categories

## Resources

- [Google Test Documentation](https://google.github.io/googletest/)
- [Google Test Primer](https://google.github.io/googletest/primer.html)
- [Google Test Advanced Guide](https://google.github.io/googletest/advanced.html)
