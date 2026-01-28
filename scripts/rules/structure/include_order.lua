-- Include directives should follow standard order:
-- 1. Standard library (<...>)
-- 2. Third-party libraries (<...>)
-- 3. Project headers ("...")
rule_description = "Include directives should follow standard order"
rule_category = "structure"

function check_ast()
    local includes = cclint.get_includes()
    if not includes then return end

    local last_type = 0  -- 0=none, 1=system, 2=local

    for _, include in ipairs(includes) do
        if include.is_system then
            if last_type == 2 then
                cclint.report_warning(
                    include.line, 1,
                    "System includes (<...>) should come before local includes (\"...\")"
                )
            end
            last_type = 1
        else
            last_type = 2
        end
    end
end
