-- Namespace names must use lowercase (e.g., mylib, utils)
rule_description = "Namespace names must use lowercase"
rule_category = "naming"

function check_ast()
    local namespaces = cclint.get_namespaces()
    if not namespaces then return end

    for _, ns in ipairs(namespaces) do
        if not ns.name or ns.name == "" then
            goto next_ns
        end

        -- Must be all lowercase with underscores
        if not ns.name:match("^[a-z][a-z0-9_]*$") then
            cclint.report_warning(
                ns.line, 1,
                string.format("Namespace name '%s' should use lowercase (e.g., mylib)", ns.name)
            )
        end

        ::next_ns::
    end
end
