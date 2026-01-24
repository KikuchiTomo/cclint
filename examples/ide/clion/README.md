# cclint CLion/IntelliJ IDEA Integration

This guide explains how to integrate cclint with JetBrains CLion or IntelliJ IDEA C++ plugin.

## Features

- **External Tools**: Run cclint from within the IDE
- **File Watchers**: Automatically run cclint on save
- **Custom Inspections**: Integrate cclint diagnostics
- **Keyboard Shortcuts**: Quick access to cclint functions

## Prerequisites

- CLion 2022.1 or later (or IntelliJ IDEA with C++ plugin)
- cclint installed and in PATH
- C++ compiler configured in CLion

## Setup

### 1. Configure External Tools

**Via UI:**

1. Open: `Settings → Tools → External Tools`
2. Click `+` to add a new tool
3. Configure each tool as shown below

**Via Configuration File:**

Copy `external_tools.xml` to your project's `.idea/tools/` directory.

#### Tool: Check Current File

- **Name**: cclint - Check Current File
- **Program**: `cclint`
- **Arguments**: `--config=$ProjectFileDir$/.cclint.yaml g++ $FilePath$`
- **Working directory**: `$ProjectFileDir$`
- **Output filters**: Add output filter with regex:
  ```
  $FILE_PATH$:$LINE$:$COLUMN$: $MESSAGE$
  ```

#### Tool: Check All Files

- **Name**: cclint - Check All Files
- **Program**: `cclint`
- **Arguments**: `--config=$ProjectFileDir$/.cclint.yaml g++ $ProjectFileDir$/src/*.cpp`
- **Working directory**: `$ProjectFileDir$`

#### Tool: Auto Fix

- **Name**: cclint - Auto Fix Current File
- **Program**: `cclint`
- **Arguments**: `--config=$ProjectFileDir$/.cclint.yaml --fix g++ $FilePath$`
- **Working directory**: `$ProjectFileDir$`
- **Check**: "Synchronize files after execution"

### 2. Configure File Watchers (Optional)

For automatic linting on save:

1. Open: `Settings → Tools → File Watchers`
2. Click `+` to add a custom watcher
3. Configure:
   - **Name**: cclint
   - **File type**: C/C++
   - **Scope**: Project Files
   - **Program**: `cclint`
   - **Arguments**: `--config=$ProjectFileDir$/.cclint.yaml --format=text g++ $FilePath$`
   - **Output paths to refresh**: `$FilePath$`
   - **Working directory**: `$ProjectFileDir$`
   - **Advanced Options**:
     - Trigger: On save
     - Create output file from stdout: No

### 3. Configure Keyboard Shortcuts

1. Open: `Settings → Keymap`
2. Search for "cclint"
3. Right-click on each tool and select "Add Keyboard Shortcut"

Suggested shortcuts:
- `Ctrl+Shift+L`: cclint - Check Current File
- `Ctrl+Shift+A`: cclint - Auto Fix Current File
- `Ctrl+Shift+P`: cclint - Profile Analysis

### 4. Configure Output Filters

To make diagnostics clickable in the Run window:

1. Run cclint once using External Tools
2. In the Run window, click the filter icon (funnel)
3. Add a new filter:
   - **Name**: cclint diagnostics
   - **Regular expression**: `^(.+):(\d+):(\d+):\s+(warning|error|info)\s+\[(.+)\]:\s+(.+)$`
   - **File**: `$1`
   - **Line**: `$2`
   - **Column**: `$3`

## Usage

### Run cclint on Current File

1. Right-click in the editor
2. Select `External Tools → cclint - Check Current File`
3. View results in the Run window
4. Click on diagnostics to jump to the location

### Run cclint on All Files

1. Right-click on the project root in Project view
2. Select `External Tools → cclint - Check All Files`
3. View results in the Run window

### Auto-Fix Issues

1. Right-click in the editor
2. Select `External Tools → cclint - Auto Fix Current File`
3. File will be automatically modified and synced

### Profile Analysis Performance

1. Right-click in the editor
2. Select `External Tools → cclint - Profile Analysis`
3. View performance statistics in the output

## Configuration

### Project Configuration

Create `.cclint.yaml` in your project root:

```yaml
version: 1.0
cpp_standard: "c++17"

lua_scripts:
  - ${CCLINT_HOME}/scripts/rules/naming.lua
  - ${CCLINT_HOME}/scripts/rules/style.lua
  - custom_rules/company_standards.lua

output:
  format: text
  show_rule_names: true
  color: false  # CLion console doesn't support colors well

rules:
  - naming-convention:
      enabled: true
      severity: warning
  - magic-numbers:
      enabled: true
      severity: info
```

### IDE Settings

Add to `CMakeLists.txt` to integrate with CLion's build system:

```cmake
# Add custom target for cclint
add_custom_target(lint
    COMMAND cclint --config=${CMAKE_SOURCE_DIR}/.cclint.yaml g++ ${CMAKE_SOURCE_DIR}/src/*.cpp
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running cclint..."
)

# Run cclint before build (optional)
add_dependencies(your_target lint)
```

Then run from CLion: `Build → lint`

## Advanced Integration

### LSP Server Integration

CLion supports Language Server Protocol. To use cclint-lsp:

1. Install cclint-lsp
2. Open: `Settings → Languages & Frameworks → Language Server Protocol → Server Definitions`
3. Add a new server:
   - **Extension/File name pattern**: `*.cpp;*.hpp;*.cc;*.h`
   - **Path to executable**: `/usr/local/bin/cclint-lsp`
   - **Args**: (leave empty)
   - **Restart policy**: On demand

Note: LSP support in CLion is experimental as of 2023.

### Custom Inspections

To create a custom inspection that runs cclint:

1. Install "Custom Inspection" plugin
2. Create inspection configuration:
   ```xml
   <inspection>
     <name>cclint</name>
     <command>cclint --config=$PROJECT_DIR$/.cclint.yaml --format=json g++ $FILE_PATH$</command>
     <output_format>json</output_format>
   </inspection>
   ```

### Code Cleanup

Add cclint auto-fix to Code Cleanup:

1. Open: `Settings → Editor → Code Style → C/C++ → Code Cleanup`
2. Add custom cleanup action:
   - **Name**: cclint auto-fix
   - **Command**: `cclint --config=$PROJECT_DIR$/.cclint.yaml --fix g++ $FILE_PATH$`

Then use: `Code → Code Cleanup → Run cclint auto-fix`

## Troubleshooting

### "cclint: command not found"

**Solution**: Add cclint to PATH or use absolute path in External Tools:

```
/usr/local/bin/cclint
```

### No Output in Run Window

**Solution**: Check "Show console when a message is printed to standard output"

### Diagnostics Not Clickable

**Solution**: Add output filter (see "Configure Output Filters" above)

### File Watcher Not Triggering

**Solution**: Check that:
1. File type matches (C/C++)
2. Scope includes the file
3. File Watchers is enabled globally

### Performance Issues

For large projects:

1. Disable File Watchers
2. Use External Tools manually or on-demand
3. Enable cclint caching in `.cclint.yaml`:
   ```yaml
   cache:
     enabled: true
   ```

## Tips and Tricks

### Quick Run

Create a run configuration:

1. `Run → Edit Configurations`
2. Add new "Shell Script" configuration:
   - **Script**: `cclint --config=.cclint.yaml g++ $FilePath$`
   - **Working directory**: Project root

### Commit Hook

Run cclint before commit:

1. `Settings → Version Control → Commit`
2. Under "Before Commit", add:
   - **Run Tool**: cclint - Check All Files
   - **Check**: "Perform code analysis"

### Multi-File Analysis

For analyzing multiple files matching a pattern:

Create external tool with:
```bash
find $ProjectFileDir$/src -name "*.cpp" -exec cclint --config=$ProjectFileDir$/.cclint.yaml g++ {} \;
```

## See Also

- [cclint Documentation](../../../docs/usage.md)
- [cclint Configuration](../../../docs/build.md)
- [Lua API Reference](../../../docs/lua_api.md)
- [CLion External Tools Documentation](https://www.jetbrains.com/help/clion/settings-tools-external-tools.html)
