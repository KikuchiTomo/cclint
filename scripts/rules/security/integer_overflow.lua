-- 整数オーバーフローの可能性を検出
local rule = {name = "integer-overflow", description = "整数オーバーフローの可能性を検出", severity = "warning"}
function rule:init(params) end
function rule:visit_binary_operator(bin_op)
    local op = bin_op:get_operator()
    if op == "+" or op == "*" then
        local lhs_type = bin_op:get_lhs():get_type()
        if lhs_type:is_integer() and lhs_type:is_signed() then
            self:report_diagnostic({severity = self.severity, message = "整数オーバーフローの可能性があります", location = bin_op:get_location(), notes = {{severity = "info", message = "チェック付き演算を使用するか、より大きな型を使用してください"}}})
        end
    end
end
return rule
