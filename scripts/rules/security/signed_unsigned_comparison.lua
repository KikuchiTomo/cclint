-- 符号付きと符号なしの比較を警告
local rule = {name = "signed-unsigned-comparison", description = "符号付きと符号なしの比較を警告", severity = "warning"}
function rule:init(params) end
function rule:visit_binary_operator(bin_op)
    local op = bin_op:get_operator()
    if op == "==" or op == "!=" or op == "<" or op == ">" or op == "<=" or op == ">=" then
        local lhs_type = bin_op:get_lhs():get_type()
        local rhs_type = bin_op:get_rhs():get_type()
        if lhs_type:is_integer() and rhs_type:is_integer() then
            if lhs_type:is_signed() ~= rhs_type:is_signed() then
                self:report_diagnostic({severity = self.severity, message = "符号付きと符号なしの比較は予期しない結果を招く可能性があります", location = bin_op:get_location()})
            end
        end
    end
end
return rule
