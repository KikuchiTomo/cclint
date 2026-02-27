-- Rule: Certain functions should only be called from main()
-- Functions starting with "only_" should only be called from main()

rule_description = "Functions starting with 'only_' should only be called from main()"
rule_category = "call-graph"

function check_ast()
    -- Get all function calls
    local calls = cclint.get_function_calls()

    if not calls then
        return
    end

    -- Check each call
    for _, call in ipairs(calls) do
        local callee = call["function"] or call.callee or call.function_name
        local caller = call.caller or call.caller_name

        -- Check if callee is a "main-only" function (starts with "only_")
        if callee and caller and string.match(callee, "^only_") then
            -- If called from a function other than main, report error
            if caller ~= "main" then
                local line = call.line or 1

                cclint.report_error(
                    line, 1,
                    string.format(
                        "Function '%s' should only be called from main(), but is called from '%s'",
                        callee, caller
                    )
                )
            end
        end
    end
end
