-- Include directives should follow standard order:
-- 1. Standard library (<...>)
-- 2. Third-party libraries (<...>)
-- 3. Project headers ("...")
rule_description = "Include directives should follow standard order"
rule_category = "structure"

function check_file()
    local includes = {}
    local last_type = 0  -- 0=none, 1=system, 2=local

    for line_num, line in ipairs(file_lines) do
        local system_include = line:match('^%s*#%s*include%s*<([^>]+)>')
        local local_include = line:match('^%s*#%s*include%s*"([^"]+)"')

        if system_include then
            if last_type == 2 then
                cclint.report_warning(
                    line_num, 1,
                    "System includes (<...>) should come before local includes (\"...\")"
                )
            end
            last_type = 1
        elseif local_include then
            last_type = 2
        end
    end
end
