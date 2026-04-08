-- Global functions must use snake_case (e.g., calculate_sum, main)
rule_description = "Global functions must use snake_case"
rule_category = "naming"

function check_ast()
    local functions = cclint.get_functions()
    if not functions then return end

    for _, func in ipairs(functions) do
        -- Skip if empty name or looks like a class method
        if not func.name or func.name == "" then
            goto next_func
        end

        -- Skip constructors/destructors
        if func.name:match("^~") or func.name:match("^[A-Z]") then
            goto next_func
        end

        -- snake_case: lowercase with underscores
        if not func.name:match("^[a-z][a-z0-9_]*$") then
            cclint.report_warning(
                func.line, 1,
                string.format("Function name '%s' should use snake_case (e.g., calculate_sum)", func.name)
            )
        end

        ::next_func::
    end
end
