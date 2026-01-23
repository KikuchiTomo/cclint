# Troubleshooting Guide

This guide helps you diagnose and fix common issues with cclint.

## Table of Contents

- [Build Issues](#build-issues)
- [Runtime Issues](#runtime-issues)
- [Configuration Issues](#configuration-issues)
- [Compiler Detection Issues](#compiler-detection-issues)
- [Output Issues](#output-issues)
- [Performance Issues](#performance-issues)
- [Getting Help](#getting-help)

## Build Issues

### CMake Not Found

**Error:**
```
bash: cmake: command not found
```

**Solution:**

Ubuntu/Debian:
```bash
sudo apt update
sudo apt install cmake
```

macOS:
```bash
brew install cmake
```

Verify installation:
```bash
cmake --version
```

### C++17 Not Supported

**Error:**
```
error: This file requires compiler and library support for the ISO C++ 2017 standard
```

**Solution:**

Check your compiler version:
```bash
g++ --version  # Should be 7.0 or higher
clang++ --version  # Should be 10.0 or higher
```

Ubuntu - Install newer GCC:
```bash
sudo apt install g++-9
export CXX=g++-9
./build.sh --clean
```

macOS - Update Xcode:
```bash
xcode-select --install
```

### Build Fails with "make: function definition file not found"

**Error:**
```
make: function definition file not found
```

**Solution:**

This can happen with certain shell configurations. Use absolute path to make:

```bash
cd build
/usr/bin/make -j8
```

Or use the build script:
```bash
./build.sh --clean
```

### Linker Errors

**Error:**
```
undefined reference to `std::filesystem::...`
```

**Solution:**

You may need to link against the filesystem library explicitly. Edit `CMakeLists.txt`:

```cmake
target_link_libraries(cclint PRIVATE stdc++fs)
```

### Permission Denied During Build

**Error:**
```
Permission denied when writing to build directory
```

**Solution:**

Ensure you have write permissions:
```bash
chmod -R u+w build/
./build.sh --clean
```

## Runtime Issues

### No Compiler Command Specified

**Error:**
```
Error: No compiler command specified
Usage: cclint [OPTIONS] <compiler-command>
```

**Solution:**

You must provide a compiler command:
```bash
# Wrong
cclint

# Correct
cclint g++ -std=c++17 main.cpp
```

### Compiler Command Not Found

**Error:**
```
Error: Failed to execute compiler command
```

**Solution:**

Make sure the compiler is in your PATH:
```bash
# Check if compiler exists
which g++
which clang++

# Add to PATH if needed
export PATH="/path/to/compiler:$PATH"
```

### Source Files Not Found

**Error:**
```
[INFO] Source files found:
# (empty list)
```

**Solution:**

1. Check that source files exist:
```bash
ls -la main.cpp
```

2. Use absolute paths:
```bash
cclint g++ /full/path/to/main.cpp
```

3. Check file extensions - only `.cpp`, `.cc`, `.cxx`, `.c`, `.h`, `.hpp` are recognized

### Exit Code 2 - Runtime Error

**Error:**
```
Error: <various error messages>
# Program exits with code 2
```

**Solution:**

Exit code 2 indicates a runtime error:
- Invalid command-line options → Check `cclint --help`
- Config file parsing error → Validate your YAML syntax
- Missing files → Verify file paths

Enable verbose mode to see details:
```bash
cclint -v g++ main.cpp
```

## Configuration Issues

### Config File Not Loaded

**Issue:**
```
[INFO] Using default configuration
```

**Solution:**

cclint searches for config files in this order:
1. `.cclint.yaml` (current directory)
2. `cclint.yaml` (current directory)
3. Parent directories
4. `~/.cclint/config.yaml`

Verify your config file:
```bash
# Check if file exists
ls -la .cclint.yaml

# Specify explicitly
cclint --config=.cclint.yaml g++ main.cpp
```

### YAML Syntax Error

**Note**: YAML parsing is not yet implemented (Milestone 2), but when it is:

**Error:**
```
Error: Failed to parse configuration file
```

**Solution:**

1. Validate YAML syntax:
```bash
# Use online validator or
python3 -c "import yaml; yaml.safe_load(open('.cclint.yaml'))"
```

2. Common YAML mistakes:
   - Incorrect indentation (use spaces, not tabs)
   - Missing colons
   - Unquoted special characters

Example of valid YAML:
```yaml
version: 1.0
lua_scripts:
  - path: "${CCLINT_HOME}/scripts/rules/naming/class-name-camelcase.lua"
    priority: 100
```

### Invalid Option Values

**Error:**
```
Error: Invalid output format: xyz
```

**Solution:**

Check valid option values:
```bash
# Valid output formats
--format=text
--format=json
--format=xml

# Invalid
--format=html  # Not supported
```

See `cclint --help` for all valid options.

## Compiler Detection Issues

### Compiler Not Detected

**Warning:**
```
[WARN] Could not detect compiler type
```

**Impact:**

This warning doesn't prevent cclint from working, but some features may be limited.

**Solution:**

1. Verify compiler is in PATH:
```bash
which g++
which clang++
```

2. Check compiler version output:
```bash
g++ --version
clang++ --version
```

3. Use full path to compiler:
```bash
cclint /usr/bin/g++ -std=c++17 main.cpp
```

### Wrong Compiler Detected

**Issue:**
```
[INFO] Detected compiler: g++ version ...
# But you're actually using clang++
```

**Solution:**

This can happen with compiler aliases. Use explicit compiler path:
```bash
cclint /usr/bin/clang++ -std=c++17 main.cpp
```

## Output Issues

### No Color in Output

**Issue:**

Text output is not colorized.

**Solution:**

1. Check if running in a terminal:
```bash
# Force color output
export FORCE_COLOR=1
cclint g++ main.cpp
```

2. Or use a different output format:
```bash
cclint --format=json g++ main.cpp
```

### JSON Output Malformed

**Issue:**

JSON output doesn't parse correctly.

**Solution:**

1. Redirect only cclint output:
```bash
cclint --format=json g++ main.cpp 2>/dev/null > output.json
```

2. Validate JSON:
```bash
python3 -m json.tool output.json
```

### XML Output Not Recognized

**Issue:**

IDE doesn't recognize XML output.

**Solution:**

cclint uses Checkstyle format. Make sure your IDE supports it:
```bash
cclint --format=xml g++ main.cpp > checkstyle-result.xml
```

Configure your IDE to read from `checkstyle-result.xml`.

## Performance Issues

### Slow Build Time

**Issue:**

cclint takes a long time to run.

**Cause:**

Currently (Milestone 1), cclint runs the actual compiler command. Build time is dominated by compilation.

**Solution:**

1. Use parallel builds:
```bash
cclint g++ -std=c++17 -j8 src/*.cpp
```

2. Use incremental builds (future feature - Milestone 4)

3. Enable caching (future feature - Milestone 4):
```yaml
# In .cclint.yaml
enable_cache: true
```

### High Memory Usage

**Cause:**

Not yet applicable (parser and rule engine not integrated).

**Future Solutions** (Milestone 4):

```yaml
# In .cclint.yaml
num_threads: 2  # Reduce parallelism
```

## Verbose Output for Debugging

Enable verbose mode to see detailed information:

```bash
cclint -v g++ -std=c++17 main.cpp
```

This shows:
- Config file loading
- Compiler detection
- Source file extraction
- Compiler flags
- Rule execution (when implemented)

For even more detail (when implemented):
```bash
cclint -vv g++ -std=c++17 main.cpp  # Very verbose
```

## Common Error Messages

### "Error: Configuration file not found"

This error will appear in Milestone 2 when YAML parsing is implemented.

**Solution:** Create a config file or use `--config` option.

### "Error: Rule script not found"

This error will appear in Milestone 3 when Lua integration is complete.

**Solution:** Check that script paths in config file are correct.

### "Error: Lua script execution failed"

This error will appear in Milestone 3.

**Solution:** Check Lua script syntax and logic.

## Known Limitations (Current Version)

**Note**: This is version 0.1.0-alpha. The following features are not yet implemented:

### Not Implemented:

- ⏳ **C++ AST Parsing** (Milestone 2)
  - Current: Compiler commands are executed but source code is not parsed
  - Future: Full AST analysis with Clang libtooling

- ⏳ **Lua Rule Execution** (Milestone 3)
  - Current: 100 Lua rule scripts exist but are not executed
  - Future: LuaJIT integration for rule execution

- ⏳ **YAML Configuration** (Milestone 2)
  - Current: Default configuration used, YAML files not parsed
  - Future: Full YAML parsing with yaml-cpp

- ⏳ **Diagnostic Reporting** (Milestones 2-3)
  - Current: No lint errors are reported (placeholder diagnostic engine)
  - Future: Real diagnostics from rule execution

- ⏳ **Fix-it Hints** (Milestone 6+)
  - Current: Not implemented
  - Future: Automatic code fixes

- ⏳ **Caching** (Milestone 4)
  - Current: No caching
  - Future: Parse result caching for faster incremental analysis

### Workarounds:

For now, cclint can be used to:
- ✅ Validate project structure
- ✅ Test compiler detection
- ✅ Verify source file extraction
- ✅ Test output format generation
- ✅ Integrate into build systems

Full linting functionality will be available in Milestone 3.

## Reporting Bugs

If you encounter a bug not covered in this guide:

1. Check [GitHub Issues](https://github.com/yourusername/cclint/issues)

2. Gather debug information:
```bash
cclint --version
cclint -v g++ --version
cclint -v g++ -std=c++17 main.cpp 2>&1 | tee debug.log
```

3. Create a minimal reproduction:
```bash
# Create minimal test case
echo '#include <iostream>
int main() { return 0; }' > test.cpp

# Try to reproduce
cclint g++ test.cpp
```

4. Report the issue with:
   - cclint version (`cclint --version`)
   - Operating system and version
   - Compiler and version
   - Full error message
   - Steps to reproduce

## Getting Help

### Documentation

- [README.md](../README.md) - Project overview
- [build.md](build.md) - Build instructions
- [usage.md](usage.md) - Usage guide
- [requirements.md](requirements.md) - Feature requirements
- [milestones.md](milestones.md) - Development roadmap
- [TODO.md](TODO.md) - Current development status

### Community

- [GitHub Issues](https://github.com/yourusername/cclint/issues) - Bug reports and feature requests
- [GitHub Discussions](https://github.com/yourusername/cclint/discussions) - Questions and discussions

### Development Status

See [TODO.md](TODO.md) for current implementation status and [WORK_SUMMARY.md](../WORK_SUMMARY.md) for completed work.
