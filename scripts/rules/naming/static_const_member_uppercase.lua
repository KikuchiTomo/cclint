-- Static const member variables should use UPPER_CASE (e.g., MAX_SIZE, DEFAULT_VALUE)
rule_description = "Static const members should use UPPER_CASE"
rule_category = "naming"

function check_file()
    local in_class = false

    for line_num, line in ipairs(file_lines) do
        -- Track class
        if line:match("^%s*class%s+") then
            in_class = true
        elseif line:match("^%s*};") and in_class then
            in_class = false
        end

        -- Check static const member declarations
        if in_class then
            -- Match: static const type VARIABLE_NAME
            if line:match("static%s+const") or line:match("const%s+static") then
                local var_name = line:match("[%s]([%w_]+)%s*[;=]")
                if var_name then
                    -- Should be UPPER_CASE
                    if not var_name:match("^[A-Z][A-Z0-9_]*$") then
                        cclint.report_warning(
                            line_num, 1,
                            string.format("Static const member '%s' should use UPPER_CASE (e.g., MAX_SIZE)", var_name)
                        )
                    end
                end
            end
        end
    end
end
