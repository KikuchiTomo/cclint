-- インクルード順序は標準的な順序に従うこと:
-- 1. 標準ライブラリ (<...>)
-- 2. サードパーティライブラリ (<...>)
-- 3. プロジェクトヘッダ ("...")
rule_description = "インクルードは標準的な順序に従うこと"
rule_category = "structure"

function check_ast()
    local includes = cclint.get_includes()
    if not includes then return end

    local last_type = 0  -- 0=なし, 1=システム, 2=ローカル

    for _, include in ipairs(includes) do
        if include.is_system then
            if last_type == 2 then
                cclint.report_warning(
                    include.line, 1,
                    "システムインクルード（<...>）はローカルインクルード（\"...\"）より先に記述してください"
                )
            end
            last_type = 1
        else
            last_type = 2
        end
    end
end
