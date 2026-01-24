# cclint Vim/Neovim Integration

This guide covers multiple ways to integrate cclint with Vim and Neovim, including:
- **ALE** (Asynchronous Lint Engine) - Async linting
- **Native LSP** (Neovim only) - Real-time analysis
- **coc.nvim** - LSP client for Vim/Neovim
- **Quickfix** - Built-in error list
- **Auto-fix** - Automatic code fixes
- **Profiling** - Performance analysis

## Prerequisites

- Vim 8.0+ or Neovim 0.5+
- cclint installed and in PATH
- cclint-lsp installed (for LSP features)

## Installation

### Option 1: Using vim-plug (Recommended)

Add to your `~/.vimrc` or `~/.config/nvim/init.vim`:

```vim
" Install plugins
Plug 'dense-analysis/ale'              " For async linting
" OR
Plug 'neoclide/coc.nvim', {'branch': 'release'}  " For LSP

" Load cclint configuration
source ~/.vim/plugin/cclint.vim
```

Then run `:PlugInstall`

### Option 2: Manual Installation

```bash
# Copy cclint.vim to your plugin directory
cp cclint.vim ~/.vim/plugin/
# Or for Neovim
cp cclint.vim ~/.config/nvim/plugin/
```

## Integration Methods

### Method 1: ALE (Async Linting)

ALE provides asynchronous linting as you type or save.

**Features:**
- Runs cclint automatically on save
- Shows diagnostics inline
- Supports auto-fix
- Works in both Vim and Neovim

**Configuration** (in `~/.vimrc`):

```vim
" Enable ALE
let g:ale_enabled = 1

" Configure cclint as C++ linter
let g:ale_linters = {
\   'cpp': ['cclint'],
\}

" Enable auto-fix
let g:ale_fixers = {
\   'cpp': ['cclint'],
\}
let g:ale_fix_on_save = 1

" Lint only on save (not while typing)
let g:ale_lint_on_save = 1
let g:ale_lint_on_text_changed = 'never'

" Navigate between errors
nmap <silent> [w <Plug>(ale_previous_wrap)
nmap <silent> ]w <Plug>(ale_next_wrap)
```

**Usage:**
- ALE runs automatically on save
- View error details: `:ALEDetail`
- Navigate errors: `[w` and `]w`
- Apply fix: `:ALEFix`
- Toggle ALE: `:ALEToggle`

### Method 2: Native LSP (Neovim Only)

Neovim's built-in LSP client provides real-time analysis using cclint-lsp.

**Features:**
- Real-time diagnostics as you type
- Go to definition (if supported)
- Code actions
- Native Neovim integration

**Configuration** (in `~/.config/nvim/init.lua`):

```lua
local lspconfig = require('lspconfig')
local configs = require('lspconfig.configs')

-- Configure cclint-lsp
if not configs.cclint then
    configs.cclint = {
        default_config = {
            cmd = {'cclint-lsp'},
            filetypes = {'cpp', 'c'},
            root_dir = lspconfig.util.root_pattern('.cclint.yaml', 'compile_commands.json'),
            settings = {},
        },
    }
end

-- Start cclint-lsp
lspconfig.cclint.setup{
    on_attach = function(client, bufnr)
        -- Keybindings
        local opts = { noremap=true, silent=true, buffer=bufnr }
        vim.keymap.set('n', 'gd', vim.lsp.buf.definition, opts)
        vim.keymap.set('n', 'K', vim.lsp.buf.hover, opts)
        vim.keymap.set('n', '<leader>ca', vim.lsp.buf.code_action, opts)
        vim.keymap.set('n', '[d', vim.diagnostic.goto_prev, opts)
        vim.keymap.set('n', ']d', vim.diagnostic.goto_next, opts)
    end
}
```

**Usage:**
- LSP runs automatically
- Show diagnostics: `:lua vim.diagnostic.open_float()`
- Navigate: `[d` and `]d`
- Code actions: `<leader>ca`

### Method 3: coc.nvim (Vim/Neovim)

coc.nvim is a powerful LSP client that works in both Vim and Neovim.

**Installation:**

```vim
Plug 'neoclide/coc.nvim', {'branch': 'release'}
```

**Configuration** (in `~/.vim/coc-settings.json` or `:CocConfig`):

```json
{
  "languageserver": {
    "cclint": {
      "command": "cclint-lsp",
      "filetypes": ["cpp", "c"],
      "rootPatterns": [".cclint.yaml", "compile_commands.json"],
      "settings": {}
    }
  }
}
```

**Usage:**
- Navigate diagnostics: `[g` and `]g`
- Show diagnostic: `:CocDiagnostics`
- Code actions: `<leader>ca`
- Restart server: `:CocRestart`

### Method 4: Quickfix (Built-in)

Use Vim's built-in quickfix list for manual linting.

**Usage:**
- Run cclint: `:CClint` or `<leader>cl`
- Run with auto-fix: `:CClintFix` or `<leader>cf`
- Run with profiling: `:CClintProfile` or `<leader>cp`
- Navigate errors: `:cnext`, `:cprevious`
- View error list: `:copen`

## Features

### Auto-Fix

Apply automatic fixes to your code:

**ALE:**
```vim
:ALEFix
```

**Manual:**
```vim
:CClintFix
```

**On save:**
```vim
let g:ale_fix_on_save = 1
```

### Profiling

View performance statistics:

```vim
:CClintProfile
```

This opens a new buffer showing:
- Analysis time
- Files analyzed
- Cache hit rate
- Memory usage

### Custom Configuration

Specify custom config file:

```vim
" In cclint.vim, modify the command:
\   'command': 'cclint --config=/path/to/config.yaml g++ %s',
```

## Keybindings

Default keybindings (defined in `cclint.vim`):

- `<leader>cl` - Run cclint on current file
- `<leader>cf` - Run cclint with auto-fix
- `<leader>cp` - Run cclint with profiling

To customize, add to your `~/.vimrc`:

```vim
nnoremap <F5> :CClint<CR>
nnoremap <F6> :CClintFix<CR>
nnoremap <F7> :CClintProfile<CR>
```

## Advanced Configuration

### Auto-run on Save

Add to `~/.vimrc`:

```vim
autocmd BufWritePost *.cpp,*.hpp,*.cc,*.h call RunCClint()
```

### Custom Error Format

For better quickfix integration:

```vim
setlocal errorformat=%f:%l:%c:\ %tarning\ [%m]:\ %m
setlocal errorformat+=%f:%l:%c:\ %trror\ [%m]:\ %m
setlocal errorformat+=%f:%l:%c:\ %tnfo\ [%m]:\ %m
```

### Disable for Certain Files

```vim
let g:ale_pattern_options = {
\   '\.min\.cpp$': {'ale_enabled': 0},
\}
```

### Multiple Linters

Use cclint alongside other linters:

```vim
let g:ale_linters = {
\   'cpp': ['cclint', 'cppcheck', 'clang'],
\}
```

## Troubleshooting

### cclint not found

**Problem:** `cclint: command not found`

**Solution:** Add cclint to PATH or use absolute path:

```vim
" In cclint.vim
\   'executable': '/usr/local/bin/cclint',
```

### LSP not starting

**Problem:** cclint-lsp server not starting

**Solution:**
1. Check if cclint-lsp is installed:
   ```bash
   which cclint-lsp
   ```

2. Check LSP logs:
   ```vim
   :LspInfo  " For nvim-lsp
   :CocInfo  " For coc.nvim
   ```

3. Verify config file exists:
   ```bash
   ls .cclint.yaml
   ```

### No diagnostics shown

**Problem:** Linter runs but no diagnostics appear

**Solution:**
1. Run cclint manually to verify it works:
   ```bash
   cclint g++ yourfile.cpp
   ```

2. Check ALE status:
   ```vim
   :ALEInfo
   ```

3. View cclint output:
   ```vim
   :CClint
   ```

### Performance issues

**Problem:** Editor slow with real-time linting

**Solution:**
1. Disable linting while typing:
   ```vim
   let g:ale_lint_on_text_changed = 'never'
   let g:ale_lint_on_insert_leave = 0
   ```

2. Use LSP instead of ALE for better performance

3. Enable cclint caching in `.cclint.yaml`:
   ```yaml
   cache:
     enabled: true
   ```

## Example Configurations

### Minimal Configuration

```vim
" Load cclint plugin
source ~/.vim/plugin/cclint.vim

" Run on save
autocmd BufWritePost *.cpp,*.hpp :CClint
```

### Full-Featured Configuration

```vim
" Plugin manager
call plug#begin()
Plug 'dense-analysis/ale'
call plug#end()

" Load cclint
source ~/.vim/plugin/cclint.vim

" ALE configuration
let g:ale_enabled = 1
let g:ale_linters = {'cpp': ['cclint']}
let g:ale_fixers = {'cpp': ['cclint']}
let g:ale_fix_on_save = 1
let g:ale_lint_on_save = 1
let g:ale_lint_on_text_changed = 'never'

" Keybindings
nnoremap <F5> :CClint<CR>
nnoremap <F6> :CClintFix<CR>
nnoremap <F7> :CClintProfile<CR>
nmap <silent> [w <Plug>(ale_previous_wrap)
nmap <silent> ]w <Plug>(ale_next_wrap)

" Status line
set statusline+=%{ALEGetStatusLine()}
```

### Neovim LSP Configuration

```lua
-- init.lua
local lspconfig = require('lspconfig')
local configs = require('lspconfig.configs')

if not configs.cclint then
    configs.cclint = {
        default_config = {
            cmd = {'cclint-lsp'},
            filetypes = {'cpp', 'c'},
            root_dir = lspconfig.util.root_pattern('.cclint.yaml'),
            settings = {},
        },
    }
end

lspconfig.cclint.setup{
    on_attach = function(client, bufnr)
        local opts = { noremap=true, silent=true, buffer=bufnr }
        vim.keymap.set('n', 'gd', vim.lsp.buf.definition, opts)
        vim.keymap.set('n', 'K', vim.lsp.buf.hover, opts)
        vim.keymap.set('n', '[d', vim.diagnostic.goto_prev, opts)
        vim.keymap.set('n', ']d', vim.diagnostic.goto_next, opts)
        vim.keymap.set('n', '<leader>ca', vim.lsp.buf.code_action, opts)
    end
}

-- Diagnostic configuration
vim.diagnostic.config({
    virtual_text = true,
    signs = true,
    update_in_insert = false,
})
```

## See Also

- [cclint Documentation](../../../docs/usage.md)
- [cclint Configuration](../../../docs/build.md)
- [ALE Documentation](https://github.com/dense-analysis/ale)
- [Neovim LSP](https://neovim.io/doc/user/lsp.html)
- [coc.nvim](https://github.com/neoclide/coc.nvim)
