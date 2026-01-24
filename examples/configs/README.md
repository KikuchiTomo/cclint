# cclint Configuration Examples

This directory contains example configuration files for common use cases.

## Available Configurations

### minimal.yaml
**Purpose**: Minimal configuration for basic linting

**Use case**: Quick setup, learning cclint, or projects with relaxed standards

**Includes**:
- Basic naming conventions
- Header guards

**Usage**:
```bash
cclint --config=examples/configs/minimal.yaml g++ -std=c++17 main.cpp
```

---

### strict.yaml
**Purpose**: Comprehensive strict configuration for high code quality

**Use case**: Large projects, team codebases, production code

**Includes**:
- All naming conventions
- Strict style enforcement
- Required documentation
- Modern C++ features mandatory
- Performance optimization required
- All security checks
- ~45 rules enabled

**Usage**:
```bash
cclint --config=examples/configs/strict.yaml g++ -std=c++17 src/*.cpp
```

---

### modern-cpp.yaml
**Purpose**: Focus on modern C++11/14/17/20 features

**Use case**: Modernizing legacy codebases, enforcing modern C++ practices

**Includes**:
- All C++11/14/17 modernization rules
- Modern initialization and syntax
- Lambda and functional programming
- Smart pointer best practices
- ~35 rules enabled

**Usage**:
```bash
cclint --config=examples/configs/modern-cpp.yaml clang++ -std=c++17 src/*.cpp
```

---

### performance.yaml
**Purpose**: Optimize for performance and efficiency

**Use case**: Performance-critical code, game development, high-frequency trading

**Includes**:
- All performance optimization rules
- Container efficiency
- String optimization
- Function inlining hints
- Copy avoidance
- ~20 rules enabled

**Usage**:
```bash
cclint --config=examples/configs/performance.yaml g++ -std=c++17 -O3 src/*.cpp
```

---

### security.yaml
**Purpose**: Security-first configuration

**Use case**: Security-sensitive applications, system software, network services

**Includes**:
- All security vulnerability checks
- Buffer overflow detection
- Unsafe function detection
- Integer overflow checks
- Race condition detection
- ~25 rules enabled

**Usage**:
```bash
cclint --config=examples/configs/security.yaml g++ -std=c++17 -Wall src/*.cpp
```

---

## Customization

You can use these configurations as templates and customize them:

1. **Copy to your project**:
```bash
cp examples/configs/modern-cpp.yaml .cclint.yaml
```

2. **Edit to add/remove rules**:
```yaml
lua_scripts:
  # Add your own rules
  - path: ".cclint/my-custom-rule.lua"
    priority: 100

  # Remove rules by commenting out
  # - path: "${CCLINT_HOME}/scripts/rules/..."
```

3. **Adjust priorities**:
Higher priority rules run first. Use this to control rule ordering:
```yaml
lua_scripts:
  - path: "rule1.lua"
    priority: 100  # Runs first
  - path: "rule2.lua"
    priority: 90   # Runs second
```

## Combining Configurations

You can combine aspects of different configurations:

```yaml
version: 1.0

# Modern C++ + Security
lua_scripts:
  # From modern-cpp.yaml
  - path: "${CCLINT_HOME}/scripts/rules/modernize/use-nullptr.lua"
    priority: 100
  - path: "${CCLINT_HOME}/scripts/rules/modernize/use-override.lua"
    priority: 100

  # From security.yaml
  - path: "${CCLINT_HOME}/scripts/rules/security/no-unsafe-functions.lua"
    priority: 100
  - path: "${CCLINT_HOME}/scripts/rules/security/buffer-overflow-risk.lua"
    priority: 95
```

## Testing Configurations

Before applying to your full codebase, test on a small file:

```bash
# Create a test file
cat > test.cpp << 'EOF'
#include <iostream>
int main() {
    int* ptr = NULL;  // Should trigger use-nullptr
    return 0;
}
EOF

# Test with different configs
cclint --config=examples/configs/minimal.yaml g++ test.cpp
cclint --config=examples/configs/modern-cpp.yaml g++ test.cpp
cclint --config=examples/configs/strict.yaml g++ test.cpp
```

## CI/CD Integration

Use these configurations in your CI/CD pipeline:

**GitHub Actions**:
```yaml
- name: Run cclint with strict config
  run: |
    cclint --config=examples/configs/strict.yaml --format=json g++ src/*.cpp
```

**GitLab CI**:
```yaml
lint:
  script:
    - cclint --config=examples/configs/security.yaml --format=xml g++ src/*.cpp
```

## Progressive Adoption

For existing projects, adopt rules progressively:

1. **Week 1**: Start with `minimal.yaml`
2. **Week 2**: Add `security.yaml` rules
3. **Week 3**: Add `modern-cpp.yaml` rules
4. **Week 4**: Gradually move to `strict.yaml`

Or create a custom progression:

```yaml
# project-week1.yaml
lua_scripts:
  - path: "${CCLINT_HOME}/scripts/rules/structure/header-guard.lua"
    priority: 100

# project-week2.yaml
lua_scripts:
  - path: "${CCLINT_HOME}/scripts/rules/structure/header-guard.lua"
    priority: 100
  - path: "${CCLINT_HOME}/scripts/rules/security/no-unsafe-functions.lua"
    priority: 100
```

## Configuration Best Practices

1. **Start minimal**: Don't enable all rules at once
2. **Test first**: Verify configurations on test files
3. **Document choices**: Add comments explaining why rules are enabled/disabled
4. **Version control**: Commit `.cclint.yaml` to your repository
5. **Team agreement**: Discuss and agree on rules with your team
6. **Regular review**: Revisit configuration quarterly

## Getting Help

- See [full list of rules](../../scripts/README.md)
- Read [usage guide](../../docs/usage.md)
- Check [troubleshooting](../../docs/troubleshooting.md)

## Creating Your Own Configuration

See the [Configuration Guide](../../docs/usage.md#configuration) for details on:
- YAML syntax
- Rule parameters
- File patterns
- Performance settings
- Output formats
