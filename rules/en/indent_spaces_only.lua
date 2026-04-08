-- Indentation must use spaces only (no tabs)
rule_description = "Indentation must use spaces only"
rule_category = "style"

function check_ast()
    local file_info = cclint.get_file_info()
    if not file_info or not file_info.lines then return end

    for line_num, line_info in pairs(file_info.lines) do
        if line_info.uses_tabs then
            cclint.report_warning(
                line_num, 1,
                "Use spaces for indentation, not tabs"
            )
        end
    end
end
