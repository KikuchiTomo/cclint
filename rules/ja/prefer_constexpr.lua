-- コンパイル時定数にはconstよりconstexprを推奨する
rule_description = "コンパイル時定数にはconstよりconstexprを推奨する"
rule_category = "readability"

function check_ast()
    local file_info = cclint.get_file_info()
    if not file_info or not file_info.lines then return end

    for line_num, line_info in pairs(file_info.lines) do
        local line = line_info.text

        -- リテラルで初期化されたconstを検出
        if line:match("const%s+") and not line:match("constexpr%s+") then
            -- リテラル値で初期化されているか確認
            if line:match("=%s*%d+") or line:match("=%s*\"") or line:match("=%s*'") or
               line:match("=%s*true") or line:match("=%s*false") then
                cclint.report_info(
                    line_num, 1,
                    "コンパイル時定数にはconstの代わりにconstexprの使用を検討してください"
                )
            end
        end
    end
end
