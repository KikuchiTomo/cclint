# Vim/Neovim Integration for cclint

This directory contains configuration examples for integrating cclint with Vim and Neovim.

## Installation

### Using vim-plug

Add to your `~/.vimrc` or `~/.config/nvim/init.vim`:

```vim
" Install ALE (Asynchronous Lint Engine)
Plug 'dense-analysis/ale'

" Load cclint configuration
source ~/.vim/plugin/cclint.vim
```

### Manual Installation

Copy `cclint.vim` to `~/.vim/plugin/` or `~/.config/nvim/plugin/`.

## Usage

### With ALE

ALE will automatically run cclint when you save C++ files.

- View errors: `:ALEDetail`
- Navigate errors: `:ALENext`, `:ALEPrevious`
- Toggle ALE: `:ALEToggle`

### Without ALE (Quickfix)

- Run cclint manually: `:CClint` or `<leader>cl`
- Navigate errors: `:cnext`, `:cprevious`
- View error list: `:copen`

## Configuration

### Custom cclint command

Edit `cclint.vim` and modify the command:

```vim
\   'command': 'cclint --config=.cclint.yaml --format=text g++ -std=c++17 %s',
```

### Enable auto-run on save

Uncomment in `cclint.vim`:

```vim
autocmd BufWritePost *.cpp,*.hpp call RunCClint()
```

## Troubleshooting

- Ensure `cclint` is in your `$PATH`
- Check ALE configuration: `:ALEInfo`
- View cclint output: Run `cclint` directly in terminal
