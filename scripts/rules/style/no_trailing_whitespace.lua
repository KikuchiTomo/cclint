-- Lines should not have trailing whitespace
rule_description = "Lines should not have trailing whitespace"
rule_category = "style"

function check_file()
    for line_num, line in ipairs(file_lines) do
        -- Check for trailing spaces or tabs
        if line:match("%s+$") then
            cclint.report_warning(
                line_num, #line,
                "Remove trailing whitespace"
            )
        end
    end
end
