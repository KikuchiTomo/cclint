" Vim integration for cclint
" Place this in ~/.vim/plugin/cclint.vim or use with vim-plug

" ALE (Asynchronous Lint Engine) configuration for cclint
if exists('g:loaded_ale')
    " Register cclint as a linter for C++
    call ale#linter#Define('cpp', {
    \   'name': 'cclint',
    \   'output_stream': 'stdout',
    \   'executable': 'cclint',
    \   'command': 'cclint --format=text g++ -std=c++17 %s',
    \   'callback': 'ale#handlers#gcc#HandleGCCFormat',
    \})

    " Enable cclint for C++ files
    let g:ale_linters = {
    \   'cpp': ['cclint'],
    \}

    " Run linters on save
    let g:ale_lint_on_save = 1
    let g:ale_lint_on_text_changed = 'never'
    let g:ale_lint_on_insert_leave = 0

    " Auto-fix support
    let g:ale_fixers = {
    \   'cpp': ['cclint'],
    \}

    " Define cclint fixer
    function! ale#fixers#cclint#Fix(buffer) abort
        return {
        \   'command': 'cclint --fix g++ -std=c++17 %t',
        \   'read_temporary_file': 1,
        \}
    endfunction

    " Enable fix on save
    let g:ale_fix_on_save = 1
endif

" LSP (Language Server Protocol) integration with coc.nvim
if exists('g:did_coc_loaded')
    " Configure cclint-lsp server
    " Add to coc-settings.json:
    " {
    "   "languageserver": {
    "     "cclint": {
    "       "command": "cclint-lsp",
    "       "filetypes": ["cpp", "c"],
    "       "rootPatterns": [".cclint.yaml", "compile_commands.json"],
    "       "settings": {}
    "     }
    "   }
    " }
endif

" Neovim LSP integration
if has('nvim') && exists('*nvim_lsp')
    lua << EOF
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

    -- Start cclint-lsp for C++ files
    lspconfig.cclint.setup{}
EOF
endif

" Quickfix integration (without ALE)
" Run cclint and populate quickfix window
function! RunCClint()
    " Save current file
    write

    " Run cclint
    let l:cmd = 'cclint --format=text g++ -std=c++17 ' . shellescape(expand('%'))
    let l:output = system(l:cmd)

    " Parse output and populate quickfix
    cexpr l:output
    copen
endfunction

" Run cclint with auto-fix
function! RunCClintFix()
    write
    let l:cmd = 'cclint --fix g++ -std=c++17 ' . shellescape(expand('%'))
    let l:output = system(l:cmd)

    " Reload file if modified
    edit!
    echo "cclint auto-fix applied"
endfunction

" Run cclint with profiling
function! RunCClintProfile()
    write
    let l:cmd = 'cclint --profile g++ -std=c++17 ' . shellescape(expand('%'))
    let l:output = system(l:cmd)

    " Show output in new buffer
    new
    setlocal buftype=nofile bufhidden=wipe noswapfile
    call setline(1, split(l:output, '\n'))
    syntax match cclintInfo /^INFO.*/
    syntax match cclintWarning /^WARN.*/
    syntax match cclintError /^ERROR.*/
    highlight cclintInfo ctermfg=blue
    highlight cclintWarning ctermfg=yellow
    highlight cclintError ctermfg=red
endfunction

" Commands
command! CClint call RunCClint()
command! CClintFix call RunCClintFix()
command! CClintProfile call RunCClintProfile()

" Keybindings
nnoremap <leader>cl :CClint<CR>
nnoremap <leader>cf :CClintFix<CR>
nnoremap <leader>cp :CClintProfile<CR>

" Auto-run cclint on save (optional)
" autocmd BufWritePost *.cpp,*.hpp call RunCClint()

" Errorformat for cclint output
" Format: filename:line:column: severity [rule-name]: message
setlocal errorformat=%f:%l:%c:\ %tarning\ [%m]:\ %m
setlocal errorformat+=%f:%l:%c:\ %trror\ [%m]:\ %m
setlocal errorformat+=%f:%l:%c:\ %tnfo\ [%m]:\ %m
