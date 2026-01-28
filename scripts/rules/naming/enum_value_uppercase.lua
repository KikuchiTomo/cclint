-- Enum values must use UPPER_CASE (e.g., ERROR_SUCCESS, MAX_VALUE)
rule_description = "Enum values must use UPPER_CASE"
rule_category = "naming"

function check_ast()
    local enums = cclint.get_enums()
    if not enums then return end

    for _, enum in ipairs(enums) do
        if not enum.values then
            goto next_enum
        end

        for _, value in ipairs(enum.values) do
            -- UPPER_CASE: uppercase with underscores
            if not value:match("^[A-Z][A-Z0-9_]*$") then
                cclint.report_warning(
                    enum.line, 1,
                    string.format("Enum value '%s' should use UPPER_CASE (e.g., ERROR_SUCCESS)", value)
                )
            end
        end

        ::next_enum::
    end
end
