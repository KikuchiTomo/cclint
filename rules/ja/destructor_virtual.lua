-- 基底クラスのデストラクタはvirtualにすること
rule_description = "基底クラスのデストラクタはvirtualにすること"
rule_category = "test"

function check_ast()
    local destructors = cclint.get_destructors()

    if not destructors then
        print("[TEST] get_destructors() が nil を返しました")
        return
    end

    print("[TEST] " .. #destructors .. " 個のデストラクタを検出")

    -- 継承ツリーを取得
    local tree = cclint.get_inheritance_tree()

    if not tree then
        print("[TEST] get_inheritance_tree() が nil を返しました")
        return
    end

    -- 基底クラス（派生クラスを持つクラス）を特定
    local base_classes = {}
    for derived, bases in pairs(tree) do
        for _, base in ipairs(bases) do
            base_classes[base] = true
        end
    end

    for _, dtor in ipairs(destructors) do
        print("[TEST] クラスのデストラクタ: " .. (dtor.class_name or "不明"))
        print("[TEST]   is_virtual: " .. tostring(dtor.is_virtual))

        -- 基底クラスかつデストラクタがvirtualでない場合
        if base_classes[dtor.class_name] then
            local info = cclint.get_destructor_info(dtor.class_name)
            if info and not info.is_virtual then
                cclint.report_error(
                    dtor.line, 1,
                    "基底クラス '" .. dtor.class_name .. "' のデストラクタはvirtualにしてください"
                )
            end
        end
    end
end
