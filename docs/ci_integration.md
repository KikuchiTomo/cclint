# CI/CD Integration Guide

This guide explains how to integrate cclint into your CI/CD pipeline.

## Table of Contents

- [GitHub Actions](#github-actions)
- [GitLab CI](#gitlab-ci)
- [Jenkins](#jenkins)
- [Best Practices](#best-practices)

---

## GitHub Actions

GitHub Actions configuration is already included in the repository at `.github/workflows/ci.yml`.

### Basic Example

```yaml
name: C++ Lint

on: [push, pull_request]

jobs:
  lint:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3

      - name: Install cclint
        run: |
          git clone https://github.com/KikuchiTomo/cclint.git /tmp/cclint
          cd /tmp/cclint
          ./build.sh
          sudo make -C build install

      - name: Run cclint
        run: |
          cclint --format=text g++ -std=c++17 src/*.cpp
```

### With JSON Output

```yaml
- name: Run cclint
  run: |
    cclint --format=json g++ -std=c++17 src/*.cpp > cclint-report.json

- name: Upload results
  uses: actions/upload-artifact@v3
  with:
    name: cclint-report
    path: cclint-report.json
```

---

## GitLab CI

A complete example is available at `examples/ci/gitlab-ci.yml`.

### Key Features

- Runs on every commit
- Generates code quality report
- Allows failure (optional)
- Caches build artifacts

### Configuration

Place `.gitlab-ci.yml` in your repository root:

```yaml
cclint:
  stage: lint
  image: gcc:latest
  script:
    - cclint --format=text g++ -std=c++17 src/*.cpp
  artifacts:
    reports:
      codequality: cclint-report.json
  allow_failure: true
```

---

## Jenkins

A complete Jenkinsfile is available at `examples/ci/Jenkinsfile`.

### Pipeline Setup

1. Create a new Pipeline job in Jenkins
2. Point it to your repository
3. Use the provided Jenkinsfile

### Key Stages

- **Checkout**: Get source code
- **Install Dependencies**: Install cclint
- **Lint**: Run cclint and parse results
- **Build**: Compile the project
- **Test**: Run tests

### Warning Parser Plugin

To visualize cclint warnings in Jenkins, install the [Warnings Next Generation Plugin](https://plugins.jenkins.io/warnings-ng/).

---

## Best Practices

### 1. Configuration File

Always use a `.cclint.yaml` configuration file for consistency:

```yaml
version: "1.0"
cpp_standard: "c++17"

rules:
  - name: naming-convention
    enabled: true
    severity: warning

  - name: max-line-length
    enabled: true
    severity: warning
    parameters:
      max_length: "100"

include_patterns:
  - "src/**/*.cpp"
  - "include/**/*.hpp"

exclude_patterns:
  - "third_party/**"
  - "build/**"
```

### 2. Exit Codes

cclint returns:
- `0`: No errors
- `1`: Errors found
- `2`: Execution error

Configure your CI to handle these appropriately:

```bash
# Fail on errors but not warnings
cclint --format=text g++ src/*.cpp || exit $?

# Always succeed (for informational purposes)
cclint --format=text g++ src/*.cpp || true
```

### 3. Incremental Linting

For large projects, lint only changed files:

```bash
# Git diff
CHANGED_FILES=$(git diff --name-only --diff-filter=d origin/main...HEAD | grep -E '\.(cpp|hpp)$')

if [ -n "$CHANGED_FILES" ]; then
  cclint --format=text g++ $CHANGED_FILES
fi
```

### 4. Output Formats

Choose the right format for your use case:

- **text**: Human-readable, good for console output
- **json**: Machine-parseable, good for tooling integration
- **xml**: Compatible with checkstyle and similar tools

### 5. Caching

Cache cclint installation to speed up builds:

**GitHub Actions:**
```yaml
- uses: actions/cache@v3
  with:
    path: /usr/local/bin/cclint
    key: cclint-${{ hashFiles('**/cclint-version.txt') }}
```

**GitLab CI:**
```yaml
cache:
  paths:
    - cclint-install/
```

### 6. Parallel Execution

For multi-file projects, consider parallel execution:

```bash
find src -name '*.cpp' | parallel -j4 cclint --format=json g++ {}
```

---

## Troubleshooting

### cclint not found

Ensure cclint is in the PATH or use absolute path:

```bash
/usr/local/bin/cclint --format=text g++ src/*.cpp
```

### Permission denied

Install with proper permissions:

```bash
sudo make -C build install
```

### No source files found

Check your compiler command includes source files:

```bash
# Correct
cclint g++ -std=c++17 src/main.cpp

# Incorrect (missing source file)
cclint g++ -std=c++17
```

### Config file not found

Specify config file explicitly:

```bash
cclint --config=path/to/.cclint.yaml g++ src/*.cpp
```

---

## Integration Examples

### Pre-commit Hook

Add to `.git/hooks/pre-commit`:

```bash
#!/bin/bash
FILES=$(git diff --cached --name-only --diff-filter=d | grep -E '\.(cpp|hpp)$')

if [ -n "$FILES" ]; then
    cclint --format=text g++ -std=c++17 $FILES
    if [ $? -ne 0 ]; then
        echo "cclint found issues. Commit aborted."
        exit 1
    fi
fi
```

### Docker Container

```dockerfile
FROM gcc:latest

# Install cclint
RUN git clone https://github.com/KikuchiTomo/cclint.git /tmp/cclint && \
    cd /tmp/cclint && \
    ./build.sh && \
    make -C build install && \
    rm -rf /tmp/cclint

WORKDIR /workspace
ENTRYPOINT ["cclint"]
```

Usage:
```bash
docker run -v $(pwd):/workspace cclint --format=text g++ src/*.cpp
```

---

## Further Reading

- [Configuration Guide](usage.md)
- [Output Format Reference](output_formats.md)
- [Rule Documentation](../scripts/README.md)
