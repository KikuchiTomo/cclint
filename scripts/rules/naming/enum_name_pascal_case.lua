-- Enum names must use PascalCase (e.g., ErrorCode, Status)
rule_description = "Enum names must use PascalCase"
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
                string.format("Enum name '%s' should use PascalCase (e.g., ErrorCode)", enum.name)
            )
        end

        ::next_enum::
    end
end
