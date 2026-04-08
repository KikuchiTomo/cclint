-- ラムダで [=] キャプチャを避けること
rule_description = "ラムダで [=] キャプチャを避けること"
rule_category = "test"

function check_ast()
    local lambdas = cclint.get_lambdas()

    if not lambdas then
        print("[TEST] get_lambdas() が nil を返しました")
        return
    end

    print("[TEST] " .. #lambdas .. " 個のラムダを検出")

    for _, lambda in ipairs(lambdas) do
        print("[TEST] ラムダ: " .. lambda.line .. "行目")
        print("[TEST]   capture_clause: " .. (lambda.capture_clause or "なし"))
        print("[TEST]   is_mutable: " .. tostring(lambda.is_mutable))

        if lambda.capture_clause and lambda.capture_clause:match("%[=") then
            cclint.report_warning(
                lambda.line, 1,
                "[=] キャプチャは避け、明示的にキャプチャしてください"
            )
        end
    end
end
