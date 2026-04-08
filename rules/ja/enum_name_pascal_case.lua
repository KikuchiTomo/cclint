-- enum名はPascalCaseを使用すること (例: ErrorCode, Status)
rule_description = "enum名はPascalCaseを使用すること"
rule_category = "naming"

function check_ast()
    local enums = cclint.get_enums()
    if not enums then return end

    for _, enum in ipairs(enums) do
        if not enum.name or enum.name == "" then
            goto next_enum
        end

        -- PascalCase: starts with uppercase
        if not enum.name:match("^[A-Z][a-zA-Z0-9]*$") then
            cclint.report_warning(
                enum.line, 1,
                string.format("enum名 '%s' はPascalCaseにしてください (例: ErrorCode)", enum.name)
            )
        end

        ::next_enum::
    end
end
