-- Lines should not exceed maximum length
rule_description = "Lines should not exceed maximum length"
rule_category = "style"

function check_file()
    local max_length = 100
    if rule_params and rule_params.max_length then
        max_length = tonumber(rule_params.max_length) or 100
    end

    for line_num, line in ipairs(file_lines) do
        if #line > max_length then
            cclint.report_warning(
                line_num, max_length + 1,
                string.format("Line exceeds maximum length of %d characters (found %d)", max_length, #line)
            )
        end
    end
end
