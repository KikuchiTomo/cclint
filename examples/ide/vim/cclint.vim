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

    " Fix on save (if cclint supports auto-fix in the future)
    " let g:ale_fix_on_save = 1
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

" Command to run cclint
command! CClint call RunCClint()

" Keybinding to run cclint
nnoremap <leader>cl :CClint<CR>

" Auto-run cclint on save (optional)
" autocmd BufWritePost *.cpp,*.hpp call RunCClint()

" Errorformat for cclint output
" Format: filename:line:column: severity [rule-name]: message
setlocal errorformat=%f:%l:%c:\ %tarning\ [%m]:\ %m
setlocal errorformat+=%f:%l:%c:\ %trror\ [%m]:\ %m
setlocal errorformat+=%f:%l:%c:\ %tnfo\ [%m]:\ %m
