-- friend 宣言の検出
rule_description = "friend 宣言の検出"
rule_category = "test"

function check_ast()
    local friends = cclint.get_friends()

    if not friends then
        print("[TEST] get_friends() が nil を返しました")
        return
    end

    print("[TEST] " .. #friends .. " 個の friend 宣言を検出")

    for _, friend in ipairs(friends) do
        print("[TEST] friend 宣言: " .. friend.line .. "行目")
        print("[TEST]   対象: " .. (friend.target or "不明"))
        print("[TEST]   種類: " .. (friend.kind or "不明"))

        cclint.report_info(
            friend.line, 1,
            "friend 宣言: " .. friend.target .. "（種類: " .. friend.kind .. "）"
        )
    end
end
