# Preprocessor Usage Guide

## Overview

The cclint preprocessor supports two modes:

1. **Linter Mode** (Default): Preserves macro names and skips includes for rule checking
2. **Expansion Mode**: Fully expands macros and includes like a standard preprocessor

## Linter Mode (Default - Recommended for Rule Checking)

In linter mode, the preprocessor:
- **Preserves macro names** instead of expanding them
- **Skips #include directives** to avoid expanding system headers
- Still processes conditional compilation (`#ifdef`, `#if`, etc.)
- Still defines macros for conditional checks

This mode is **essential for naming convention rules** because:
- You can check if macro names follow conventions (e.g., `ALL_CAPS`)
- You don't need to parse massive system headers (iostream, vector, etc.)
- Analysis remains focused on user code

### Example: Linter Mode

```cpp
#include <iostream>
#define MAX_SIZE 100
#define calculate(x) ((x) * 2)

int main() {
    int size = MAX_SIZE;
    int result = calculate(10);
    return 0;
}
```

**Linter Mode Output:**
```cpp
// #include is skipped (not expanded)
// Macros are preserved as identifiers
int main() {
    int size = MAX_SIZE;      // <- "MAX_SIZE" preserved for naming checks
    int result = calculate(10); // <- "calculate" preserved for naming checks
    return 0;
}
```

**Usage:**
```cpp
Preprocessor pp(source, filename);
// Don't set any flags (defaults are false)
auto tokens = pp.preprocess();

// Now you can check:
// - Is "MAX_SIZE" in ALL_CAPS? ✓
// - Is "calculate" lowercase? ✓
```

## Expansion Mode (For Full Preprocessing)

In expansion mode, the preprocessor behaves like a traditional C/C++ preprocessor.

### Example: Expansion Mode

```cpp
#define MAX_SIZE 100
#define calculate(x) ((x) * 2)

int main() {
    int size = MAX_SIZE;
    int result = calculate(10);
    return 0;
}
```

**Expansion Mode Output:**
```cpp
int main() {
    int size = 100;           // <- MAX_SIZE expanded to 100
    int result = ((10) * 2);  // <- calculate(10) expanded to ((10) * 2)
    return 0;
}
```

**Usage:**
```cpp
Preprocessor pp(source, filename);
pp.set_expand_macros(true);     // Enable macro expansion
pp.set_expand_includes(true);   // Enable include expansion
auto tokens = pp.preprocess();

// Now tokens contain fully expanded code
```

## Selective Include Expansion

You can expand user includes while skipping system includes:

```cpp
#include <iostream>      // <- System include (skipped)
#include "myheader.h"    // <- User include (expanded)

int main() {
    return 0;
}
```

**Usage:**
```cpp
Preprocessor pp(source, filename);
pp.set_expand_includes(true);           // Enable includes
pp.set_expand_system_includes(false);   // But skip system headers
auto tokens = pp.preprocess();

// Result: "myheader.h" is expanded, but <iostream> is skipped
```

## API Reference

### Constructor
```cpp
Preprocessor(const std::string& source,
             const std::string& filename = "",
             const std::vector<std::string>& include_paths = {});
```

### Configuration Methods

#### `set_expand_macros(bool expand)`
- **Default:** `false` (linter mode)
- **true:** Expand all macro invocations
- **false:** Preserve macro names as identifiers

```cpp
pp.set_expand_macros(true);   // Enable expansion
pp.set_expand_macros(false);  // Disable expansion (default)
```

#### `set_expand_includes(bool expand)`
- **Default:** `false` (linter mode)
- **true:** Process `#include` directives and insert file contents
- **false:** Skip `#include` directives

```cpp
pp.set_expand_includes(true);   // Enable includes
pp.set_expand_includes(false);  // Disable includes (default)
```

#### `set_expand_system_includes(bool expand)`
- **Default:** `false`
- **true:** Expand system includes (`<iostream>`)
- **false:** Skip system includes
- **Note:** Only applies when `expand_includes` is `true`

```cpp
pp.set_expand_system_includes(true);   // Expand system headers
pp.set_expand_system_includes(false);  // Skip system headers (default)
```

### Other Methods

#### `define_macro(const std::string& definition)`
Define a macro from command line (like `-D` option):
```cpp
pp.define_macro("MAX=100");
pp.define_macro("DEBUG");
```

#### `undefine_macro(const std::string& name)`
Undefine a macro (like `-U` option):
```cpp
pp.undefine_macro("NDEBUG");
```

#### `add_include_path(const std::string& path)`
Add a directory to the include search path:
```cpp
pp.add_include_path("/usr/local/include");
pp.add_include_path("./include");
```

## Comparison: Linter Mode vs Expansion Mode

| Feature | Linter Mode (Default) | Expansion Mode |
|---------|----------------------|----------------|
| Macro Expansion | ❌ Disabled | ✅ Enabled |
| Include Expansion | ❌ Disabled | ✅ Enabled |
| Conditional Compilation | ✅ Processed | ✅ Processed |
| Macro Definitions | ✅ Stored | ✅ Stored |
| Naming Rule Checks | ✅ Possible | ❌ Impossible |
| Token Count | Small (user code only) | Large (includes system headers) |
| Processing Speed | Fast | Slow |
| Use Case | Linting, style checking | Full preprocessing |

## Recommended Settings

### For Linting (Recommended)
```cpp
Preprocessor pp(source, filename);
// Use defaults - no need to set anything
auto tokens = pp.preprocess();
```

### For Build System Integration
```cpp
Preprocessor pp(source, filename, include_paths);
pp.set_expand_macros(true);
pp.set_expand_includes(true);
pp.set_expand_system_includes(true);
auto tokens = pp.preprocess();
```

### For Partial Analysis (User Code Only)
```cpp
Preprocessor pp(source, filename, include_paths);
pp.set_expand_macros(false);      // Keep macro names
pp.set_expand_includes(true);     // Expand user includes
pp.set_expand_system_includes(false);  // Skip system includes
auto tokens = pp.preprocess();
```

## Implementation Notes

### Why Defaults Are False

The preprocessor defaults to linter mode (all expansions disabled) because:

1. **Naming Convention Rules Require Original Names**
   - Macro names like `MAX_SIZE` need to be checked against naming rules
   - After expansion, you only see `100` - the name is lost

2. **System Headers Are Huge**
   - `#include <iostream>` expands to thousands of lines
   - Most linting rules don't need to check standard library code
   - Parsing system headers is slow and memory-intensive

3. **User Code Is the Focus**
   - Linters typically only check user-written code
   - System headers are already validated by the compiler

### Conditional Compilation Always Processed

Even in linter mode, conditional compilation (`#ifdef`, `#if`, etc.) is always processed because:
- It determines which code paths are active
- Linters need to know which code to analyze
- This doesn't interfere with naming checks

Example:
```cpp
#ifdef DEBUG
#define LOG(x) std::cout << x
#else
#define LOG(x)
#endif
```

Even in linter mode, the preprocessor will:
- Evaluate `#ifdef DEBUG`
- Include/exclude code blocks accordingly
- Store macro definitions for later checks
- But won't expand `LOG(x)` invocations (preserves the name)

## Future Enhancements

Planned improvements:
- [ ] Line continuation support (`\` at end of line)
- [ ] `#pragma once` optimization
- [ ] Include guards detection
- [ ] Partial macro expansion (expand some, preserve others)
- [ ] Macro usage tracking for unused macro warnings
