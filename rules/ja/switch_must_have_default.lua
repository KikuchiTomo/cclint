-- switch文にはdefaultケースが必要
rule_description = "switch文にはdefaultケースが必要"
rule_category = "readability"

function check_ast()
    local switches = cclint.get_switches()
    if not switches then return end

    for _, switch_stmt in ipairs(switches) do
        if not switch_stmt.has_default then
            cclint.report_warning(
                switch_stmt.line, 1,
                "switch文にはdefaultケースを記述してください"
            )
        end
    end
end
