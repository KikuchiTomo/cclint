-- Indentation must use spaces only (no tabs)
rule_description = "Indentation must use spaces only"
rule_category = "style"

function check_file()
    for line_num, line in ipairs(file_lines) do
        -- Check if line starts with tab
        if line:match("^%s*\t") then
            cclint.report_warning(
                line_num, 1,
                "Use spaces for indentation, not tabs"
            )
        end
    end
end
