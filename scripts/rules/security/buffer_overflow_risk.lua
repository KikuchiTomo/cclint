-- バッファオーバーフローのリスクを検出
local rule = {name = "buffer-overflow-risk", description = "バッファオーバーフローのリスクを検出", severity = "error"}
function rule:init(params) end
function rule:visit_call_expr(call_expr)
    local func_name = call_expr:get_callee():get_name()
    local risky_functions = {"strncpy", "strncat", "snprintf"}
    for _, risky in ipairs(risky_functions) do
        if func_name == risky then
            self:report_diagnostic({severity = self.severity, message = string.format("関数 '%s' は正しく使用しないとバッファオーバーフローを引き起こす可能性があります", func_name), location = call_expr:get_location()})
        end
    end
end
return rule
