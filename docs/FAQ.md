# Frequently Asked Questions (FAQ)

## General Questions

### What is cclint?

cclint is a customizable C++ linter that allows you to define custom lint rules using LuaJIT scripts without recompiling the tool itself. It wraps your existing compiler commands and provides extensible rule-based code analysis.

### Why another C++ linter?

Most C++ linters have fixed rules or require recompilation to add new rules. cclint is designed to be:
- **Customizable**: Define any rule using Lua scripts
- **Flexible**: Works with your existing compiler workflow
- **Extensible**: 100+ Lua APIs for comprehensive AST access
- **Fast**: Uses LuaJIT for near-native performance

### What C++ standards are supported?

cclint supports C++98 through C++26. The tool automatically detects the C++ standard from your compiler flags (e.g., `-std=c++17`).

### What compilers are supported?

cclint works with:
- GCC 7+ (Linux, macOS)
- Clang 10+ (Linux, macOS)
- Apple Clang (macOS)
- MSVC (Windows, planned)

### What platforms are supported?

Currently supported:
- Ubuntu 20.04 LTS or later
- macOS 11 (Big Sur) or later

Planned:
- Windows 10/11 (future milestone)

## Installation and Setup

### How do I install cclint?

**From source** (current method):
```bash
git clone https://github.com/yourusername/cclint.git
cd cclint
./build.sh
sudo make -C build install
```

**Pre-built packages** (coming soon):
```bash
# Ubuntu
sudo apt install cclint

# macOS
brew install cclint
```

### Where are the Lua rule scripts installed?

After installation:
- **Ubuntu/Linux**: `/usr/local/share/cclint/scripts/rules/`
- **macOS**: `/usr/local/share/cclint/scripts/rules/`

You can reference them in your config with `${CCLINT_HOME}`.

### Do I need to install LLVM/Clang separately?

Not yet. The current alpha version (0.1.0) doesn't require LLVM/Clang. Full AST parsing will be added in Milestone 2, which will require:
- LLVM/Clang 14.0+
- yaml-cpp 0.7.0+
- LuaJIT 2.1+

## Usage

### How do I use cclint?

Simply prefix your compiler command with `cclint`:

```bash
# Before
g++ -std=c++17 main.cpp

# After
cclint g++ -std=c++17 main.cpp
```

### How do I create a configuration file?

Create `.cclint.yaml` in your project root:

```yaml
version: 1.0

lua_scripts:
  - path: "${CCLINT_HOME}/scripts/rules/naming/class-name-camelcase.lua"
    priority: 100
```

See [examples/configs/](../examples/configs/) for templates.

### Can I use cclint without a config file?

Yes! cclint will use default settings if no config file is found. However, custom rules require a configuration file.

### How do I enable only specific rules?

List only the rules you want in your `.cclint.yaml`:

```yaml
lua_scripts:
  - path: "${CCLINT_HOME}/scripts/rules/modernize/use-nullptr.lua"
  - path: "${CCLINT_HOME}/scripts/rules/security/no-unsafe-functions.lua"
```

### How do I disable a specific rule?

Simply don't include it in your configuration file. There's no "disable" flag needed.

### What output formats are supported?

- **text**: Human-readable with ANSI colors (default)
- **json**: Machine-readable for CI/CD integration
- **xml**: Checkstyle-compatible for IDE integration

Use `--format=<format>` to specify:
```bash
cclint --format=json g++ main.cpp
```

## Rules and Customization

### How many built-in rules are available?

cclint includes **100 standard Lua rule scripts** across 9 categories:
- Naming (9 rules)
- Style (8 rules)
- Structure (4 rules)
- Spacing (5 rules)
- Documentation (4 rules)
- Modernize (18 rules)
- Performance (15 rules)
- Readability (19 rules)
- Security (11 rules)

See [scripts/README.md](../scripts/README.md) for the complete list.

### Can I write my own custom rules?

Yes! (Note: Lua integration coming in Milestone 3)

Create a Lua file:
```lua
local rule = {
    name = "my-custom-rule",
    description = "My custom check",
    severity = "warning"
}

function rule:visit_function_decl(func_decl)
    if func_decl:get_name():len() > 50 then
        self:report_diagnostic({
            message = "Function name too long",
            location = func_decl:get_location()
        })
    end
end

return rule
```

Add to your config:
```yaml
lua_scripts:
  - path: ".cclint/my-custom-rule.lua"
```

### Can I modify the standard rules?

You can copy a standard rule and modify it:
```bash
cp /usr/local/share/cclint/scripts/rules/naming/class-name-camelcase.lua \
   .cclint/my-class-naming.lua
# Edit my-class-naming.lua
```

Then use your modified version in your config.

### What Lua APIs are available?

100+ APIs for AST access, including:
- `visit_function_decl()` - Visit function declarations
- `visit_class_decl()` - Visit class declarations
- `visit_var_decl()` - Visit variable declarations
- `get_location()` - Get source location
- `get_name()` - Get identifier name
- `report_diagnostic()` - Report issues

Full API documentation coming in Milestone 3.

## CI/CD Integration

### How do I use cclint in CI/CD?

**GitHub Actions**:
```yaml
- name: Run cclint
  run: cclint --format=json g++ src/*.cpp
```

**GitLab CI**:
```yaml
lint:
  script:
    - cclint --format=xml g++ src/*.cpp
```

See [docs/usage.md](usage.md#cicd-integration) for more examples.

### What exit codes does cclint use?

- `0`: Success (no errors)
- `1`: Lint errors found
- `2`: Runtime error (invalid options, config error, etc.)

Use in scripts:
```bash
if cclint g++ main.cpp; then
    echo "No errors"
else
    echo "Errors found"
    exit 1
fi
```

### Can I fail the build on warnings?

Currently, cclint only uses exit code 1 for errors, not warnings. You can:
1. Set rule severity to "error" in config
2. Parse JSON/XML output to check warning count

This will be improved in future versions.

## Performance

### Is cclint fast?

Current performance (Milestone 1):
- cclint runs your compiler command as-is
- Overhead is minimal (< 100ms startup time)

Future performance (Milestone 4):
- Parallel file processing
- Parse result caching
- Incremental analysis

### Can I speed up cclint?

Tips for Milestone 4 and beyond:
```yaml
# In .cclint.yaml
enable_cache: true
num_threads: 8  # Or 0 for auto-detect
parallel_rules: true
```

### Does cclint cache results?

Not yet. Caching will be added in Milestone 4.

## Troubleshooting

### cclint says "command not found"

Make sure cclint is in your PATH:
```bash
which cclint
# If not found:
export PATH="/usr/local/bin:$PATH"
```

Or use the full path:
```bash
/usr/local/bin/cclint g++ main.cpp
```

### My compiler isn't detected

cclint auto-detects GCC, Clang, MSVC, and Apple Clang. If detection fails:
- Verify compiler is in PATH: `which g++`
- Use full path: `cclint /usr/bin/g++ main.cpp`
- Check compiler version: `g++ --version`

### Configuration file not loaded

cclint searches for config files in this order:
1. `.cclint.yaml` (current directory)
2. Parent directories
3. `~/.cclint/config.yaml`

Verify with verbose mode:
```bash
cclint -v g++ main.cpp
# Look for "Loaded config from: ..."
```

### I get "Parser not implemented" errors

The current alpha version (0.1.0) has limited functionality:
- ✅ Compiler wrapping works
- ✅ Configuration loading works
- ⏳ AST parsing coming in Milestone 2
- ⏳ Lua rule execution coming in Milestone 3

See [WORK_SUMMARY.md](../WORK_SUMMARY.md) for current status.

### Where can I report bugs?

Please report bugs on [GitHub Issues](https://github.com/yourusername/cclint/issues).

Include:
- cclint version (`cclint --version`)
- Operating system and version
- Compiler and version
- Minimal reproduction case
- Full error message

## Compatibility

### Does cclint work with CMake projects?

Yes! Add a custom target:
```cmake
add_custom_target(lint
    COMMAND cclint ${CMAKE_CXX_COMPILER} ${SOURCES}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
```

### Does cclint work with Make projects?

Yes! Add to your Makefile:
```makefile
lint:
    cclint $(CXX) $(CXXFLAGS) $(SOURCES)
```

### Does cclint work with existing linters?

Yes! cclint complements tools like:
- clang-tidy
- cppcheck
- cpplint

You can run them together:
```bash
cclint g++ main.cpp && clang-tidy main.cpp
```

### Can I use cclint with header-only libraries?

Yes! cclint works with any C++ code that your compiler can process.

## Development and Contributing

### How can I contribute?

See [CONTRIBUTING.md](../CONTRIBUTING.md) for guidelines.

Ways to contribute:
- Report bugs
- Suggest features
- Write Lua rules
- Improve documentation
- Submit code patches

### What's the development roadmap?

Six milestones planned:
1. **MVP** (85% complete) - Basic functionality
2. **Rule System** - Parser integration
3. **Lua Integration** - LuaJIT and rule execution
4. **Performance** - Optimization and caching
5. **Tool Integration** - IDE and CI/CD support
6. **Release** - v1.0.0 stable release

See [docs/milestones.md](milestones.md) for details.

### How is cclint developed?

cclint is developed with assistance from Claude Code (Anthropic's AI).

See [CLAUDE.md](../CLAUDE.md) for development guidelines and [WORK_SUMMARY.md](../WORK_SUMMARY.md) for progress.

### What license is cclint under?

To be determined. The license will be open source.

## Advanced Topics

### Can I use cclint programmatically?

Not yet. A C++ API may be added in future versions. Current usage is command-line only.

### Does cclint support plugins?

Not yet. Plugin support may be added in Milestone 6+.

### Can I use cclint as a Language Server?

Not yet. LSP support is planned for a future milestone.

### How does cclint compare to clang-tidy?

| Feature | cclint | clang-tidy |
|---------|--------|------------|
| Custom rules | Lua scripts (no recompilation) | C++ code (requires recompilation) |
| Rule language | Lua (dynamic) | C++ (static) |
| Performance | Good (LuaJIT) | Excellent (native) |
| AST access | 100+ Lua APIs | Full Clang AST |
| Setup | Easy | Moderate |
| Rule count | 100+ standard rules | 400+ checks |

**When to use cclint**:
- Project-specific rules
- Rapid rule prototyping
- Lua scripting expertise
- No C++ build environment

**When to use clang-tidy**:
- Maximum performance
- Standard checks sufficient
- C++ expertise
- Deep AST manipulation

## Getting Help

### Where can I get more help?

- **Documentation**: Check [docs/](.)
- **Examples**: See [examples/configs/](../examples/configs/)
- **Troubleshooting**: Read [troubleshooting.md](troubleshooting.md)
- **Issues**: Report on [GitHub](https://github.com/yourusername/cclint/issues)
- **Discussions**: Ask on [GitHub Discussions](https://github.com/yourusername/cclint/discussions)

### Is there a community chat?

Not yet. We may set up Discord/Slack if there's interest.

### How do I stay updated?

- Watch the GitHub repository
- Check [CHANGELOG.md](../CHANGELOG.md) for updates
- Follow release announcements

---

**Didn't find your question?** Please ask on [GitHub Discussions](https://github.com/yourusername/cclint/discussions) or open an issue.
