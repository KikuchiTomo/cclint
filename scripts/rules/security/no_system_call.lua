-- system()関数の使用を禁止
--
-- 説明:
--   system()関数はコマンドインジェクションの脆弱性を引き起こす可能性があります。
--   より安全な代替手段を使用すべきです。
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度

local rule = {
    name = "no-system-call",
    description = "system()関数の使用を禁止",
    severity = "error",
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

    if func_name == "system" then
        self:report_diagnostic({
            severity = self.severity,
            message = "system()関数の使用は禁止されています",
            location = call_expr:get_location(),
            notes = {
                {
                    severity = "info",
                    message = "より安全な代替手段（exec系関数、std::system with validation等）を使用してください",
                }
            }
        })
    end
end

return rule
