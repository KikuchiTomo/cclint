# Output Format Reference

cclint supports multiple output formats for different use cases.

## Available Formats

- [Text](#text-format) (default)
- [JSON](#json-format)
- [XML](#xml-format)

---

## Text Format

Human-readable output suitable for console display.

### Usage

```bash
cclint --format=text g++ src/main.cpp
```

### Output Example

```
src/main.cpp:10:5: warning [naming-convention]: Function name 'DoSomething' does not follow snake_case convention
src/main.cpp:15:1: error [max-line-length]: Line exceeds maximum length of 80 characters (current: 120)
src/header.hpp:1:1: warning [header-guard]: Header file missing header guard or #pragma once

Analysis complete
Errors: 1
Warnings: 2
```

### Format Specification

```
<file>:<line>:<column>: <severity> [<rule-name>]: <message>
```

Fields:
- **file**: Source file path (absolute or relative)
- **line**: Line number (1-indexed)
- **column**: Column number (1-indexed)
- **severity**: `error`, `warning`, or `info`
- **rule-name**: Name of the violated rule
- **message**: Diagnostic message

### Color Output

Text format includes ANSI color codes:
- **Errors**: Red
- **Warnings**: Yellow
- **Info**: Blue
- **File paths**: Bold

To disable colors:
```bash
cclint --format=text g++ src/main.cpp 2>&1 | cat
```

---

## JSON Format

Machine-readable format for tool integration.

### Usage

```bash
cclint --format=json g++ src/main.cpp > report.json
```

### Output Example

```json
{
  "version": "1.0",
  "diagnostics": [
    {
      "file": "src/main.cpp",
      "line": 10,
      "column": 5,
      "severity": "warning",
      "rule": "naming-convention",
      "message": "Function name 'DoSomething' does not follow snake_case convention"
    },
    {
      "file": "src/main.cpp",
      "line": 15,
      "column": 1,
      "severity": "error",
      "rule": "max-line-length",
      "message": "Line exceeds maximum length of 80 characters (current: 120)"
    }
  ],
  "summary": {
    "total_files": 2,
    "total_diagnostics": 2,
    "errors": 1,
    "warnings": 1,
    "info": 0
  }
}
```

### Schema

```json
{
  "version": "string",
  "diagnostics": [
    {
      "file": "string",
      "line": "integer",
      "column": "integer",
      "severity": "error|warning|info",
      "rule": "string",
      "message": "string",
      "fix_hints": [
        {
          "range": {
            "start": {"line": "integer", "column": "integer"},
            "end": {"line": "integer", "column": "integer"}
          },
          "replacement": "string"
        }
      ]
    }
  ],
  "summary": {
    "total_files": "integer",
    "total_diagnostics": "integer",
    "errors": "integer",
    "warnings": "integer",
    "info": "integer"
  }
}
```

### Parsing JSON

**Python:**
```python
import json

with open('report.json') as f:
    data = json.load(f)

for diag in data['diagnostics']:
    print(f"{diag['file']}:{diag['line']}: {diag['message']}")
```

**JavaScript:**
```javascript
const fs = require('fs');
const report = JSON.parse(fs.readFileSync('report.json', 'utf8'));

report.diagnostics.forEach(diag => {
    console.log(`${diag.file}:${diag.line}: ${diag.message}`);
});
```

---

## XML Format

XML output compatible with checkstyle and similar tools.

### Usage

```bash
cclint --format=xml g++ src/main.cpp > report.xml
```

### Output Example

```xml
<?xml version="1.0" encoding="UTF-8"?>
<checkstyle version="1.0">
  <file name="src/main.cpp">
    <error line="10" column="5" severity="warning" message="Function name 'DoSomething' does not follow snake_case convention" source="naming-convention"/>
    <error line="15" column="1" severity="error" message="Line exceeds maximum length of 80 characters (current: 120)" source="max-line-length"/>
  </file>
  <file name="src/header.hpp">
    <error line="1" column="1" severity="warning" message="Header file missing header guard or #pragma once" source="header-guard"/>
  </file>
</checkstyle>
```

### Schema

The XML output follows the [Checkstyle format](https://checkstyle.org/):

```xml
<checkstyle version="1.0">
  <file name="string">
    <error
      line="integer"
      column="integer"
      severity="error|warning|info"
      message="string"
      source="string"/>
  </file>
</checkstyle>
```

### Parsing XML

**Python (with lxml):**
```python
from lxml import etree

tree = etree.parse('report.xml')
for error in tree.xpath('//error'):
    file = error.getparent().get('name')
    line = error.get('line')
    message = error.get('message')
    print(f"{file}:{line}: {message}")
```

---

## Format Comparison

| Feature | Text | JSON | XML |
|---------|------|------|-----|
| Human-readable | ✅ | ❌ | ⚠️ |
| Machine-parseable | ⚠️ | ✅ | ✅ |
| Color output | ✅ | ❌ | ❌ |
| CI/CD integration | ✅ | ✅ | ✅ |
| IDE integration | ✅ | ⚠️ | ⚠️ |
| Statistics | ✅ | ✅ | ❌ |
| Fix hints | ⚠️ | ✅ | ⚠️ |

---

## Use Cases

### Console/Terminal
```bash
cclint --format=text g++ src/*.cpp
```
Best for: Manual development, debugging

### CI/CD Pipelines
```bash
cclint --format=json g++ src/*.cpp > cclint-report.json
```
Best for: Automated builds, artifact storage

### IDE Integration
```bash
cclint --format=text g++ src/main.cpp 2>&1 | your-ide-parser
```
Best for: Real-time linting in editors

### Quality Dashboards
```bash
cclint --format=xml g++ src/*.cpp > checkstyle-result.xml
```
Best for: SonarQube, Jenkins, GitLab CI quality reports

---

## Exit Codes

cclint returns different exit codes based on results:

| Exit Code | Meaning |
|-----------|---------|
| 0 | Success (no errors) |
| 1 | Errors found |
| 2 | Execution error (invalid config, missing file, etc.) |

### CI/CD Behavior

```bash
# Fail build on errors
cclint --format=json g++ src/*.cpp
if [ $? -eq 1 ]; then
    echo "Linting errors found!"
    exit 1
fi

# Continue on warnings
cclint --format=json g++ src/*.cpp || true
```

---

## Custom Parsing

### Text Format Regex

```regex
^(.+):(\d+):(\d+):\s+(warning|error|info)\s+\[(.+)\]:\s+(.+)$
```

Groups:
1. File path
2. Line number
3. Column number
4. Severity
5. Rule name
6. Message

### Example (Python)

```python
import re

pattern = r'^(.+):(\d+):(\d+):\s+(warning|error|info)\s+\[(.+)\]:\s+(.+)$'

with open('cclint-output.txt') as f:
    for line in f:
        match = re.match(pattern, line)
        if match:
            file, line, col, severity, rule, msg = match.groups()
            print(f"Found {severity} in {file}:{line}")
```

---

## Future Formats

Planned for future releases:
- **SARIF**: Static Analysis Results Interchange Format
- **GitHub Actions**: Native GitHub annotations
- **JUnit**: For test integration
- **HTML**: Interactive web report

---

## Further Reading

- [Configuration Guide](usage.md)
- [CI/CD Integration](ci_integration.md)
- [IDE Integration](ide_integration.md)
