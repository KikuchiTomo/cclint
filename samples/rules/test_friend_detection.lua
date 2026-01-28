-- Test: Friend API - detect friend declarations
rule_description = "Detect friend declarations"
rule_category = "test"

function check_ast()
    local friends = cclint.get_friends()

    if not friends then
        print("[TEST] get_friends() returned nil")
        return
    end

    print("[TEST] Found " .. #friends .. " friend declarations")

    for _, friend in ipairs(friends) do
        print("[TEST] Friend at line " .. friend.line)
        print("[TEST]   target: " .. (friend.target or "unknown"))
        print("[TEST]   kind: " .. (friend.kind or "unknown"))

        cclint.report_info(
            friend.line, 1,
            "Friend declaration: " .. friend.target .. " (kind: " .. friend.kind .. ")"
        )
    end
end
