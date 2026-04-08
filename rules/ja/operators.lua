-- 演算子オーバーロードの検出
rule_description = "演算子オーバーロードの検出"
rule_category = "test"

function check_ast()
    local operators = cclint.get_operators()

    if not operators then
        print("[TEST] get_operators() が nil を返しました")
        return
    end

    print("[TEST] " .. #operators .. " 個の演算子オーバーロードを検出")

    for _, op in ipairs(operators) do
        print("[TEST] 演算子: " .. (op.operator_symbol or "不明"))
        print("[TEST]   is_member: " .. tostring(op.is_member))

        cclint.report_info(
            op.line, 1,
            "演算子オーバーロード: " .. op.operator_symbol
        )
    end
end
