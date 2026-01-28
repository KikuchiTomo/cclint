-- Test: Lambda API - avoid [=] capture
rule_description = "Avoid [=] capture in lambdas"
rule_category = "test"

function check_ast()
    local lambdas = cclint.get_lambdas()

    if not lambdas then
        print("[TEST] get_lambdas() returned nil")
        return
    end

    print("[TEST] Found " .. #lambdas .. " lambdas")

    for _, lambda in ipairs(lambdas) do
        print("[TEST] Lambda at line " .. lambda.line)
        print("[TEST]   capture_clause: " .. (lambda.capture_clause or "none"))
        print("[TEST]   is_mutable: " .. tostring(lambda.is_mutable))

        if lambda.capture_clause and lambda.capture_clause:match("%[=") then
            cclint.report_warning(
                lambda.line, 1,
                "Avoid [=] capture, use explicit captures instead"
            )
        end
    end
end
