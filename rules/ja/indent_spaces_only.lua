-- インデントにはスペースのみ使用（タブ禁止）
rule_description = "インデントにはスペースのみ使用"
rule_category = "style"

function check_ast()
    local file_info = cclint.get_file_info()
    if not file_info or not file_info.lines then return end

    for line_num, line_info in pairs(file_info.lines) do
        if line_info.uses_tabs then
            cclint.report_warning(
                line_num, 1,
                "インデントにはタブではなくスペースを使用してください"
            )
        end
    end
end
