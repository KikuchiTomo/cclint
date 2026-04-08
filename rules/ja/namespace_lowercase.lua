-- 名前空間は小文字を使用すること (例: mylib, utils)
rule_description = "名前空間は小文字を使用すること"
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
                string.format("名前空間 '%s' は小文字にしてください (例: mylib)", ns.name)
            )
        end

        ::next_ns::
    end
end
