-- restricted_function は main からのみ呼び出し可能
rule_description = "restricted_function は main からのみ呼び出し可能"
rule_category = "test"

function check_ast()
    local callers = cclint.get_callers("restricted_function")

    if not callers then
        print("[TEST] get_callers() が nil を返しました")
        return
    end

    print("[TEST] restricted_function の呼び出し元: " .. #callers .. " 個")

    for _, caller in ipairs(callers) do
        print("[TEST] 呼び出し元: " .. caller)

        if caller ~= "main" then
            -- 行番号を取得するため全関数呼び出しを確認
            local calls = cclint.get_function_calls()
            if calls then
                for _, call in ipairs(calls) do
                    if call["function"] == "restricted_function" and call.caller == caller then
                        cclint.report_error(
                            call.line, 1,
                            "restricted_function は main からのみ呼び出し可能ですが、'" .. caller .. "' から呼び出されています"
                        )
                    end
                end
            end
        end
    end
end
