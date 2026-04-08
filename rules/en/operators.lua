-- Test: Operator API
rule_description = "Detect operator overloads"
rule_category = "test"

function check_ast()
    local operators = cclint.get_operators()

    if not operators then
        print("[TEST] get_operators() returned nil")
        return
    end

    print("[TEST] Found " .. #operators .. " operator overloads")

    for _, op in ipairs(operators) do
        print("[TEST] Operator: " .. (op.operator_symbol or "unknown"))
        print("[TEST]   is_member: " .. tostring(op.is_member))

        cclint.report_info(
            op.line, 1,
            "Operator overload: " .. op.operator_symbol
        )
    end
end
