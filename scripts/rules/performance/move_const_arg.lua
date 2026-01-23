-- const引数へのstd::moveは無意味
--
-- 説明:
--   const修飾された引数にstd::moveを適用しても効果がありません。
--   constを削除するか、std::moveを削除すべきです。
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度

local rule = {
    name = "move-const-arg",
    description = "const引数へのstd::moveは無意味",
    severity = "warning",
}

function rule:init(params)
    -- パラメータなし
end

function rule:visit_call_expr(call_expr)
    local callee = call_expr:get_callee()

    if not callee:is_function_decl() then
        return
    end

    local func_name = callee:get_name()

    -- std::moveのチェック
    if func_name ~= "move" or not callee:is_in_std_namespace() then
        return
    end

    -- 引数をチェック
    local args = call_expr:get_arguments()
    if #args == 0 then
        return
    end

    local arg = args[1]
    local arg_type = arg:get_type()

    -- const修飾されているかチェック
    if arg_type:is_const_qualified() then
        self:report_diagnostic({
            severity = self.severity,
            message = "const修飾された値にstd::moveを適用しても効果がありません",
            location = call_expr:get_location(),
            notes = {
                {
                    severity = "info",
                    message = "constを削除するか、std::moveを削除してください",
                }
            }
        })
    end
end

return rule
