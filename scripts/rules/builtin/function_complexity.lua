-- Function Complexity Rule (Lua version, replaces C++ builtin)
rule_description = "Check cyclomatic complexity of functions"
rule_category = "readability"

function check_ast()
    local max_complexity = 10
    if rule_params and rule_params.max_complexity then
        max_complexity = tonumber(rule_params.max_complexity) or 10
    end

    local functions = cclint.get_functions()
    if not functions then
        return
    end

    for i, func in ipairs(functions) do
        -- AST doesn't calculate complexity yet, so we'll use a simple approximation
        -- based on function length (this is a placeholder)
        -- TODO: Implement actual cyclomatic complexity calculation

        -- For now, just report if function exists (demonstration)
        -- Real implementation would calculate based on control flow
    end
end
