-- グローバル関数はsnake_caseを使用すること (例: calculate_sum, main)
rule_description = "グローバル関数はsnake_caseを使用すること"
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
                string.format("関数名 '%s' はsnake_caseにしてください (例: calculate_sum)", func.name)
            )
        end

        ::next_func::
    end
end
