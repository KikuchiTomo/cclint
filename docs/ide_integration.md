# IDE Integration Guide

This guide shows how to integrate cclint with popular IDEs and text editors.

## Table of Contents

- [Visual Studio Code](#visual-studio-code)
- [Vim/Neovim](#vimneovim)
- [CLion](#clion)
- [Sublime Text](#sublime-text)
- [Emacs](#emacs)

---

## Visual Studio Code

### Installation

1. Copy example configurations from `examples/ide/vscode/`:
   - `tasks.json` → `.vscode/tasks.json`
   - `settings.json` → `.vscode/settings.json`

### Tasks

Run cclint tasks from the Command Palette (`Ctrl+Shift+P` / `Cmd+Shift+P`):

- **Tasks: Run Task** → **cclint: Lint Current File**
- **Tasks: Run Task** → **cclint: Lint All Files**

### Keyboard Shortcuts

Add to `keybindings.json`:

```json
{
    "key": "ctrl+shift+l",
    "command": "workbench.action.tasks.runTask",
    "args": "cclint: Lint Current File"
}
```

### Problem Matcher

The provided `tasks.json` includes a problem matcher that:
- Parses cclint output
- Shows diagnostics in the Problems panel
- Enables click-to-navigate to issues

### Auto-lint on Save

Install [Task Runner extension](https://marketplace.visualstudio.com/items?itemName=SanaAjani.taskrunnercode) and configure:

```json
{
    "taskRunner.autoRun": {
        "on": "save",
        "task": "cclint: Lint Current File"
    }
}
```

---

## Vim/Neovim

### With ALE (Recommended)

1. Install [ALE](https://github.com/dense-analysis/ale):

   ```vim
   " Using vim-plug
   Plug 'dense-analysis/ale'
   ```

2. Copy `examples/ide/vim/cclint.vim` to `~/.vim/plugin/` or `~/.config/nvim/plugin/`

3. Configuration in `.vimrc` or `init.vim`:

   ```vim
   " Enable cclint for C++
   let g:ale_linters = {'cpp': ['cclint']}

   " Lint on save only
   let g:ale_lint_on_save = 1
   let g:ale_lint_on_text_changed = 'never'

   " Navigate errors
   nmap <silent> [e <Plug>(ale_previous_wrap)
   nmap <silent> ]e <Plug>(ale_next_wrap)
   ```

### Without ALE (Quickfix)

Use the provided `:CClint` command:

```vim
" Run cclint
:CClint

" Navigate errors
:cnext
:cprevious
:copen
:cclose

" Keybindings
nnoremap <leader>cl :CClint<CR>
nnoremap <leader>cn :cnext<CR>
nnoremap <leader>cp :cprevious<CR>
```

### Auto-run on Save

Add to your config:

```vim
autocmd BufWritePost *.cpp,*.hpp call RunCClint()
```

---

## CLion

### External Tool

1. Open **Settings** → **Tools** → **External Tools**
2. Click **+** to add a new tool
3. Configure:
   - **Name**: cclint
   - **Program**: `cclint`
   - **Arguments**: `--format=text g++ -std=c++17 $FilePath$`
   - **Working directory**: `$ProjectFileDir$`

### File Watcher

1. Install **File Watchers** plugin (if not already installed)
2. Open **Settings** → **Tools** → **File Watchers**
3. Click **+** to add a custom watcher
4. Configure:
   - **Name**: cclint
   - **File type**: C++
   - **Scope**: Project Files
   - **Program**: `cclint`
   - **Arguments**: `--format=text g++ -std=c++17 $FilePath$`
   - **Output paths to refresh**: `$FilePath$`

### Output Parser

Configure the error pattern in File Watcher settings:

```
$FILE_PATH$:$LINE$:$COLUMN$: $MESSAGE$
```

---

## Sublime Text

### Build System

Create `cclint.sublime-build`:

```json
{
    "shell_cmd": "cclint --format=text g++ -std=c++17 ${file}",
    "file_regex": "^(.+):(\\d+):(\\d+): (warning|error|info) \\[(.+)\\]: (.+)$",
    "selector": "source.c++",
    "variants": [
        {
            "name": "Lint All",
            "shell_cmd": "cclint --config=.cclint.yaml --format=text g++ -std=c++17 ${project_path}/src/*.cpp"
        }
    ]
}
```

### Usage

1. **Tools** → **Build System** → **cclint**
2. Press `Ctrl+B` / `Cmd+B` to lint current file
3. Press `Ctrl+Shift+B` / `Cmd+Shift+B` to select variant (Lint All)

### With SublimeLinter

Install [SublimeLinter](http://www.sublimelinter.com/) and create a custom linter plugin.

---

## Emacs

### Flycheck Configuration

Add to your `.emacs` or `init.el`:

```elisp
(require 'flycheck)

;; Define cclint checker
(flycheck-define-checker cclint
  "A C++ linter using cclint."
  :command ("cclint"
            "--format=text"
            "g++"
            "-std=c++17"
            source)
  :error-patterns
  ((warning line-start (file-name) ":" line ":" column ": warning [" (id (one-or-more not-newline)) "]: " (message) line-end)
   (error line-start (file-name) ":" line ":" column ": error [" (id (one-or-more not-newline)) "]: " (message) line-end)
   (info line-start (file-name) ":" line ":" column ": info [" (id (one-or-more not-newline)) "]: " (message) line-end))
  :modes (c++-mode))

;; Add to flycheck checkers list
(add-to-list 'flycheck-checkers 'cclint)

;; Enable for C++ mode
(add-hook 'c++-mode-hook
          (lambda ()
            (flycheck-select-checker 'cclint)
            (flycheck-mode)))
```

### Manual Run

```elisp
(defun run-cclint ()
  "Run cclint on current buffer."
  (interactive)
  (compile (concat "cclint --format=text g++ -std=c++17 " buffer-file-name)))

(global-set-key (kbd "C-c l") 'run-cclint)
```

---

## Best Practices

### 1. Project Configuration

Keep a `.cclint.yaml` in your project root for consistent linting across IDEs:

```yaml
version: "1.0"
cpp_standard: "c++17"
include_patterns:
  - "src/**/*.cpp"
  - "include/**/*.hpp"
exclude_patterns:
  - "build/**"
  - "third_party/**"
```

### 2. Editor Settings

Configure your editor to:
- Run linter on save (not on every keystroke)
- Show inline error messages
- Highlight error lines
- Provide quick-fix suggestions (if supported)

### 3. Performance

For large projects:
- Lint only current file during development
- Run full project lint before committing
- Use incremental linting where possible

### 4. Integration with Other Tools

cclint works well alongside:
- **clang-format**: Code formatting
- **clang-tidy**: Static analysis
- **cppcheck**: Additional checks

Configure your IDE to run all tools in sequence.

---

## Troubleshooting

### Linter Not Running

- Check cclint is in PATH: `which cclint`
- Verify permissions: `ls -l $(which cclint)`
- Check configuration syntax
- View IDE/editor logs for errors

### Incorrect File Paths

Ensure working directory is set to project root in your IDE configuration.

### No Diagnostics Shown

- Verify problem matcher/error pattern is correct
- Test cclint manually in terminal
- Check if files match include patterns in `.cclint.yaml`

### Performance Issues

- Disable auto-lint on typing
- Use lint-on-save instead
- Exclude large directories (build/, third_party/)

---

## Further Reading

- [Configuration Guide](usage.md)
- [CI/CD Integration](ci_integration.md)
- [Output Format Reference](output_formats.md)
