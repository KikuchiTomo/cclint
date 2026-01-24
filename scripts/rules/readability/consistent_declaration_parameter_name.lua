-- 宣言と定義でパラメータ名を統一すべき
--
-- 説明:
--   関数の宣言と定義でパラメータ名が異なると混乱を招きます。
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度

local rule = {
    name = "consistent-declaration-parameter-name",
    description = "宣言と定義でパラメータ名を統一すべき",
    severity = "warning",
}

function rule:init(params)
    -- パラメータなし
end

function rule:visit_function_decl(func_decl)
    -- 定義のみチェック
    if not func_decl:has_body() then
        return
    end

    -- 対応する宣言を取得
    local declaration = func_decl:get_previous_declaration()
    if not declaration then
        return
    end

    local def_params = func_decl:get_parameters()
    local decl_params = declaration:get_parameters()

    -- パラメータ数が一致しない場合はスキップ
    if #def_params ~= #decl_params then
        return
    end

    -- パラメータ名を比較
    for i = 1, #def_params do
        local def_name = def_params[i]:get_name()
        local decl_name = decl_params[i]:get_name()

        -- 両方に名前がある場合のみチェック
        if def_name ~= "" and decl_name ~= "" and def_name ~= decl_name then
            self:report_diagnostic({
                severity = self.severity,
                message = string.format(
                    "パラメータ名が宣言と定義で異なります（宣言: '%s'、定義: '%s'）",
                    decl_name, def_name
                ),
                location = def_params[i]:get_location(),
                notes = {
                    {
                        severity = "info",
                        message = string.format(
                            "宣言は %s にあります",
                            declaration:get_location():to_string()
                        ),
                        location = decl_params[i]:get_location(),
                    }
                }
            })
        end
    end
end

return rule
