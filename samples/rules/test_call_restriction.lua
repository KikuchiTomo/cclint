-- Test: Call graph API - restricted_function should only be called from main
rule_description = "restricted_function should only be called from main"
rule_category = "test"

function check_ast()
    local callers = cclint.get_callers("restricted_function")

    if not callers then
        print("[TEST] get_callers() returned nil")
        return
    end

    print("[TEST] restricted_function is called by " .. #callers .. " functions")

    for _, caller in ipairs(callers) do
        print("[TEST] Caller: " .. caller)

        if caller ~= "main" then
            -- Get all function calls to find the line number
            local calls = cclint.get_function_calls()
            if calls then
                for _, call in ipairs(calls) do
                    if call["function"] == "restricted_function" and call.caller == caller then
                        cclint.report_error(
                            call.line, 1,
                            "restricted_function can only be called from main, but called from '" .. caller .. "'"
                        )
                    end
                end
            end
        end
    end
end
