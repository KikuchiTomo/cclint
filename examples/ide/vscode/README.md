# cclint VSCode Extension

This extension integrates cclint C++ Linter with Visual Studio Code, providing real-time code analysis through the Language Server Protocol (LSP).

## Features

- **Real-time Analysis**: Get instant feedback as you type using cclint-lsp
- **On-Save Linting**: Automatically run cclint when you save files
- **Problem Reporting**: See diagnostics in the Problems panel
- **Quick Fixes**: Apply automatic fixes for certain issues (if available)
- **Customizable Rules**: Configure cclint rules through `.cclint.yaml`

## Requirements

- cclint installed and available in PATH
- cclint-lsp installed for real-time analysis (optional but recommended)
- C++ compiler (g++ or clang++)

## Installation

### Install cclint

```bash
# From source
git clone https://github.com/KikuchiTomo/cclint.git
cd cclint
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
sudo make install
```

### Install VSCode Extension

1. Copy this directory to `~/.vscode/extensions/cclint-vscode-0.1.0/`
2. Restart VSCode
3. The extension will activate automatically for C++ files

## Configuration

### Workspace Settings

Create `.vscode/settings.json` in your project:

```json
{
  "cclint.enable": true,
  "cclint.configFile": "${workspaceFolder}/.cclint.yaml",
  "cclint.runOnSave": true,
  "cclint.lsp.enabled": true,
  "cclint.lsp.path": "/usr/local/bin/cclint-lsp"
}
```

### cclint Configuration

Create `.cclint.yaml` in your project root:

```yaml
version: 1.0
cpp_standard: "c++17"

lua_scripts:
  - /usr/local/share/cclint/scripts/rules/naming.lua
  - /usr/local/share/cclint/scripts/rules/style.lua

output:
  format: text
  show_rule_names: true
```

## Usage

### Real-time Analysis

With LSP enabled, diagnostics appear automatically as you type:

1. Open a C++ file
2. The cclint-lsp server starts automatically
3. Diagnostics appear in the editor and Problems panel

### Manual Run

Run cclint on the current file:

1. Open Command Palette (Ctrl+Shift+P / Cmd+Shift+P)
2. Type "cclint: Run on Current File"
3. Results appear in the integrated terminal

### Restart LSP Server

If the LSP server becomes unresponsive:

1. Open Command Palette
2. Type "cclint: Restart LSP Server"

## Keyboard Shortcuts

You can add custom keyboard shortcuts in `keybindings.json`:

```json
[
  {
    "key": "ctrl+shift+l",
    "command": "cclint.runOnCurrentFile",
    "when": "editorTextFocus && editorLangId == cpp"
  },
  {
    "key": "ctrl+shift+r",
    "command": "cclint.restart"
  }
]
```

## Troubleshooting

### LSP Server Not Starting

Check the Output panel (View → Output → cclint Language Server):

```
[Error] Failed to start cclint-lsp: command not found
```

**Solution**: Ensure cclint-lsp is in your PATH:

```bash
which cclint-lsp
# If not found, add to PATH or specify full path in settings
```

### No Diagnostics Shown

1. Check that `.cclint.yaml` exists
2. Verify cclint works from command line:
   ```bash
   cclint g++ yourfile.cpp
   ```
3. Check the LSP server logs:
   ```bash
   tail -f /tmp/cclint-lsp.log
   ```

### Performance Issues

For large projects, consider:

1. Enable caching:
   ```yaml
   # In .cclint.yaml
   cache:
     enabled: true
   ```

2. Use incremental analysis:
   ```yaml
   incremental: true
   ```

3. Adjust thread count:
   ```yaml
   num_threads: 4
   ```

## Features in Detail

### Real-time Diagnostics

The LSP server provides:
- Syntax checking
- Style violations
- Naming convention checks
- Security issues
- Performance warnings

### Quick Fixes

Some diagnostics support automatic fixes:

1. Hover over the diagnostic
2. Click "Quick Fix" or press Ctrl+. (Cmd+.)
3. Select the fix to apply

### Customization

Control which rules run:

```yaml
# .cclint.yaml
lua_rules:
  - name: naming-convention
    enabled: true
    severity: warning
  - name: magic-numbers
    enabled: false
```

## Contributing

Report issues or contribute at:
https://github.com/KikuchiTomo/cclint

## License

Same as cclint main project (see LICENSE file).

## Changelog

### 0.1.0 (2026-01-24)

- Initial release
- LSP server integration
- Real-time diagnostics
- On-save linting
- Problem matcher support
