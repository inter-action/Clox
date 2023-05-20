" load config using
" :source .vimrc

" format current file
" nnoremap <leader>ff :wa <bar> %! clang-format<CR>   

lua << EOF

vim.keymap.set('n', '<leader>ff', function()
    vim.api.nvim_command("wa")
    vim.api.nvim_command("%! clang-format")
end)

EOF