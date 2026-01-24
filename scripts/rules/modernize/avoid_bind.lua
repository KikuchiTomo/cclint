-- std::bindよりラムダを使用すべき
local rule = {name = "avoid-bind", description = "std::bindよりラムダを使用すべき", severity = "info"}
function rule:init(params) end
function rule:visit_call_expr(call_expr)
    local func_name = call_expr:get_callee():get_name()
    if func_name == "bind" and call_expr:get_callee():is_in_std_namespace() then
        self:report_diagnostic({severity = self.severity, message = "std::bindよりラムダを使用すべきです", location = call_expr:get_location(), notes = {{severity = "info", message = "ラムダの方が読みやすく、最適化されやすいです"}}})
    end
end
return rule
