-- 単一引数のコンストラクタにはexplicitを付けること
rule_description = "単一引数のコンストラクタにはexplicitが必要"
rule_category = "test"

function check_ast()
    local constructors = cclint.get_constructors()

    if not constructors then
        print("[TEST] get_constructors() が nil を返しました")
        return
    end

    print("[TEST] " .. #constructors .. " 個のコンストラクタを検出")

    for _, ctor in ipairs(constructors) do
        print("[TEST] クラスのコンストラクタ: " .. (ctor.class_name or "不明"))
        print("[TEST]   is_explicit: " .. tostring(ctor.is_explicit))
        print("[TEST]   is_default: " .. tostring(ctor.is_default))
        print("[TEST]   is_delete: " .. tostring(ctor.is_delete))

        -- 単一引数のコンストラクタか確認
        local info = cclint.get_constructor_info(ctor.class_name)
        if info then
            -- defaultでもdeleteでもない場合、explicitを確認
            if not info.is_default and not info.is_delete then
                if not info.is_explicit then
                    cclint.report_warning(
                        ctor.line, 1,
                        "クラス '" .. ctor.class_name .. "' の単一引数コンストラクタにはexplicitを付けてください"
                    )
                end
            end
        end
    end
end
