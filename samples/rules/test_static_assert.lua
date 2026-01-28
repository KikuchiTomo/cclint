-- Test: Static Assert API
rule_description = "Detect static_assert"
rule_category = "test"

function check_ast()
    local asserts = cclint.get_static_asserts()

    if not asserts then
        print("[TEST] get_static_asserts() returned nil")
        return
    end

    print("[TEST] Found " .. #asserts .. " static_assert declarations")

    for _, sa in ipairs(asserts) do
        print("[TEST] static_assert at line " .. sa.line)
        print("[TEST]   condition: " .. (sa.condition or "unknown"))
        cclint.report_info(
            sa.line, 1,
            "static_assert: " .. sa.condition
        )
    end
end
