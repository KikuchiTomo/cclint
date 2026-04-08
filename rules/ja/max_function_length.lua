-- 関数は最大行数を超えてはならない
rule_description = "関数は最大行数を超えてはならない"
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
                    string.format("関数 '%s' は %d 行あります (上限 %d 行)。リファクタリングを検討してください。",
                                  func.name or "unknown", length, max_lines)
                )
            end
        end
    end
end
