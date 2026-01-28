-- Namespace names must use lowercase (e.g., mylib, utils)
rule_description = "Namespace names must use lowercase"
rule_category = "naming"

function check_file()
    for line_num, line in ipairs(file_lines) do
        -- Match: namespace NamespaceName
        local ns_name = line:match("^%s*namespace%s+([%w_]+)")
        if ns_name then
            -- Must be all lowercase
            if not ns_name:match("^[a-z][a-z0-9_]*$") then
                cclint.report_warning(
                    line_num, 1,
                    string.format("Namespace name '%s' should use lowercase (e.g., mylib)", ns_name)
                )
            end
        end
    end
end
