-- Switch statements must have a default case
rule_description = "Switch statements must have a default case"
rule_category = "readability"

function check_ast()
    local switches = cclint.get_switches()
    if not switches then return end

    for _, switch_stmt in ipairs(switches) do
        if not switch_stmt.has_default then
            cclint.report_warning(
                switch_stmt.line, 1,
                "Switch statement should have a default case"
            )
        end
    end
end
