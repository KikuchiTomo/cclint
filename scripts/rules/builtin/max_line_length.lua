-- Max Line Length Rule (Lua version, replaces C++ builtin)
rule_description = "Check maximum line length"
rule_category = "style"

function check_file()
    local max_length = 80
    if rule_params and rule_params.max_length then
        max_length = tonumber(rule_params.max_length) or 80
    end

    for line_num, line in ipairs(file_lines) do
        if #line > max_length then
            cclint.report_warning(
                line_num,
                max_length + 1,
                string.format("Line exceeds maximum length of %d (found %d characters)",
                              max_length, #line)
            )
        end
    end
end
