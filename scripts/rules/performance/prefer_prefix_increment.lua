-- 前置インクリメントを優先すべき
local rule = {name = "prefer-prefix-increment", description = "前置インクリメントを優先すべき", severity = "info"}
function rule:init(params) end
function rule:visit_unary_operator(unary_op)
    if unary_op:is_postfix() and (unary_op:get_operator() == "++" or unary_op:get_operator() == "--") then
        local expr_type = unary_op:get_sub_expr():get_type()
        if not expr_type:is_builtin() then
            self:report_diagnostic({severity = self.severity, message = "オブジェクトには前置インクリメント/デクリメントを使用すべきです", location = unary_op:get_location()})
        end
    end
end
return rule
