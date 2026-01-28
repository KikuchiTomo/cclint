-- Lines should not exceed maximum length (default: 100 characters)
rule_description = "Lines should not exceed maximum length"
rule_category = "style"

function check_ast()
    local max_length = 100
    if rule_parameters and rule_parameters.max_length then
        max_length = tonumber(rule_parameters.max_length) or 100
    end

    local file_info = cclint.get_file_info()
    if not file_info or not file_info.lines then return end

    for line_num, line_info in pairs(file_info.lines) do
        if line_info.length > max_length then
            cclint.report_warning(
                line_num, 1,
                string.format("Line exceeds maximum length of %d characters (%d characters)",
                    max_length, line_info.length)
            )
        end
    end
end
