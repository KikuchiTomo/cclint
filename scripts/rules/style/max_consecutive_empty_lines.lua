-- Limit consecutive empty lines (default: 2)
rule_description = "Limit consecutive empty lines"
rule_category = "style"

function check_ast()
    local max_empty_lines = 2
    if rule_parameters and rule_parameters.max_empty_lines then
        max_empty_lines = tonumber(rule_parameters.max_empty_lines) or 2
    end

    local file_info = cclint.get_file_info()
    if not file_info or not file_info.lines then return end

    local empty_count = 0
    local first_empty_line = 0

    for line_num = 1, #file_lines do
        local line_info = file_info.lines[line_num]
        if line_info and line_info.is_empty then
            if empty_count == 0 then
                first_empty_line = line_num
            end
            empty_count = empty_count + 1
        else
            if empty_count > max_empty_lines then
                cclint.report_info(
                    first_empty_line, 1,
                    string.format("Too many consecutive empty lines (%d, max: %d)",
                        empty_count, max_empty_lines)
                )
            end
            empty_count = 0
        end
    end
end
