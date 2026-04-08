-- static_assert の検出
rule_description = "static_assert の検出"
rule_category = "test"

function check_ast()
    local asserts = cclint.get_static_asserts()

    if not asserts then
        print("[TEST] get_static_asserts() が nil を返しました")
        return
    end

    print("[TEST] " .. #asserts .. " 個の static_assert を検出")

    for _, sa in ipairs(asserts) do
        print("[TEST] static_assert: " .. sa.line .. "行目")
        print("[TEST]   条件: " .. (sa.condition or "不明"))
        cclint.report_info(
            sa.line, 1,
            "static_assert: " .. sa.condition
        )
    end
end
