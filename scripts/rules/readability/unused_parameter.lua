-- 未使用のパラメータに名前を付けるべきでない
--
-- 説明:
--   使用されていないパラメータは名前を削除するか、
--   [[maybe_unused]]属性を付けるべきです。
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度
--   - allow_in_virtual: 仮想関数では許可 (デフォルト: true)

local rule = {
    name = "unused-parameter",
    description = "未使用のパラメータに名前を付けるべきでない",
    severity = "info",
}

function rule:init(params)
    self.allow_in_virtual = params.allow_in_virtual ~= false
end

function rule:visit_function_decl(func_decl)
    -- 宣言のみの場合はスキップ
    if not func_decl:has_body() then
        return
    end

    -- 仮想関数のチェック
    if self.allow_in_virtual and func_decl:is_virtual() then
        return
    end

    local body = func_decl:get_body()

    for _, param in ipairs(func_decl:get_parameters()) do
        local param_name = param:get_name()

        -- 名前がないパラメータはスキップ
        if param_name == "" then
            goto continue
        end

        -- maybe_unused属性があればスキップ
        if param:has_attribute("maybe_unused") then
            goto continue
        end

        -- パラメータが使用されているかチェック
        if not self:is_parameter_used(body, param) then
            self:report_diagnostic({
                severity = self.severity,
                message = string.format(
                    "パラメータ '%s' は使用されていません",
                    param_name
                ),
                location = param:get_location(),
                notes = {
                    {
                        severity = "info",
                        message = "パラメータ名を削除するか、[[maybe_unused]]属性を追加してください",
                    }
                }
            })
        end

        ::continue::
    end
end

function rule:is_parameter_used(stmt, param)
    for _, child in ipairs(stmt:get_children()) do
        -- パラメータへの参照をチェック
        if child:is_decl_ref() and child:get_referenced_decl() == param then
            return true
        end

        -- 再帰的にチェック
        if self:is_parameter_used(child, param) then
            return true
        end
    end

    return false
end

return rule
