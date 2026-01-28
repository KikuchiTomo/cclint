-- Limit consecutive empty lines
rule_description = "Limit consecutive empty lines"
rule_category = "style"

function check_file()
    local max_empty = 2
    if rule_params and rule_params.max_empty_lines then
        max_empty = tonumber(rule_params.max_empty_lines) or 2
    end

    local empty_count = 0
    local first_empty_line = 0

    for line_num, line in ipairs(file_lines) do
        if line:match("^%s*$") then
            if empty_count == 0 then
                first_empty_line = line_num
            end
            empty_count = empty_count + 1

            if empty_count > max_empty then
                cclint.report_warning(
                    line_num, 1,
                    string.format("Too many consecutive empty lines (max %d allowed)", max_empty)
                )
            end
        else
            empty_count = 0
        end
    end
end
