-- 行末の空白を禁止
rule_description = "行末に空白を残さないこと"
rule_category = "style"

function check_ast()
    local file_info = cclint.get_file_info()
    if not file_info or not file_info.lines then return end

    for line_num, line_info in pairs(file_info.lines) do
        if line_info.has_trailing_space then
            cclint.report_info(
                line_num, 1,
                "行末に空白があります"
            )
        end
    end
end
