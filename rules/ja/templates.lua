-- テンプレートの検出
rule_description = "テンプレートの検出"
rule_category = "test"

function check_ast()
    local templates = cclint.get_templates()

    if not templates then
        print("[TEST] get_templates() が nil を返しました")
        return
    end

    print("[TEST] " .. #templates .. " 個のテンプレートを検出")

    for _, tmpl in ipairs(templates) do
        print("[TEST] テンプレート: " .. tmpl.line .. "行目")
        cclint.report_info(
            tmpl.line, 1,
            "テンプレート宣言を検出"
        )
    end
end
