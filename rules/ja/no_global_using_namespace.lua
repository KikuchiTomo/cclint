-- グローバルスコープでのusing namespaceの使用を禁止する（特にヘッダファイル）
rule_description = "グローバルスコープでusing namespaceを使用しない"
rule_category = "restrictions"

function check_ast()
    local usings = cclint.get_usings()
    if not usings then return end

    for _, using_decl in ipairs(usings) do
        -- グローバルスコープのusing namespaceを検出
        if using_decl.target and using_decl.target:match("namespace") then
            cclint.report_warning(
                using_decl.line, 1,
                "グローバルスコープでusing namespaceを使用しないでください。グローバル名前空間が汚染されます。"
            )
        end
    end
end
