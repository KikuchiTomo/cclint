-- nullptr比較は明示的に行うべき
--
-- 説明:
--   ポインタのnullチェックは明示的に!= nullptrで行うべきです。
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度

local rule = {
    name = "prefer-nullptr-comparison",
    description = "nullptr比較は明示的に行うべき",
    severity = "info",
}

function rule:init(params)
    -- パラメータなし
end

function rule:visit_if_stmt(if_stmt)
    local cond = if_stmt:get_condition()

    -- 条件がポインタの直接評価かチェック
    if cond:is_decl_ref() or cond:is_member_expr() then
        local cond_type = cond:get_type()

        if cond_type:is_pointer() then
            self:report_diagnostic({
                severity = self.severity,
                message = "ポインタのnullチェックは明示的に行うべきです",
                location = cond:get_location(),
                notes = {
                    {
                        severity = "info",
                        message = "!= nullptr を使用してください",
                    }
                }
            })
        end
    end
end

return rule
