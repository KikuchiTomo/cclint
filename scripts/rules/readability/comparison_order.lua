-- 比較の順序を統一すべき（定数を左に）
local rule = {name = "comparison-order", description = "比較の順序を統一すべき（定数を左に）", severity = "info"}
function rule:init(params) self.constant_left = params.constant_left or false end
function rule:visit_binary_operator(bin_op)
    local op = bin_op:get_operator()
    if op == "==" or op == "!=" then
        local lhs = bin_op:get_lhs()
        local rhs = bin_op:get_rhs()
        if self.constant_left and lhs:is_variable() and rhs:is_literal() then
            self:report_diagnostic({severity = self.severity, message = "比較では定数を左側に配置すべきです", location = bin_op:get_location()})
        end
    end
end
return rule
