-- enum値はUPPER_CASEを使用すること (例: ERROR_SUCCESS, MAX_VALUE)
rule_description = "enum値はUPPER_CASEを使用すること"
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
                    string.format("enum値 '%s' はUPPER_CASEにしてください (例: ERROR_SUCCESS)", value)
                )
            end
        end

        ::next_enum::
    end
end
