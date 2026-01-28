-- Functions should not exceed maximum number of lines
rule_description = "Functions should not exceed maximum number of lines"
rule_category = "readability"

function check_ast()
    local max_lines = 50
    if rule_params and rule_params.max_lines then
        max_lines = tonumber(rule_params.max_lines) or 50
    end

    local functions = cclint.get_functions()
    if not functions then return end

    for _, func in ipairs(functions) do
        if func.end_line and func.line then
            local length = func.end_line - func.line + 1
            if length > max_lines then
                cclint.report_warning(
                    func.line, 1,
                    string.format("Function '%s' is %d lines long (max %d allowed). Consider refactoring.",
                                  func.name or "unknown", length, max_lines)
                )
            end
        end
    end
end
