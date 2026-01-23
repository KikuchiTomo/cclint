-- 透過的な比較演算子を使用すべき
local rule = {name = "use-transparent-comparators", description = "透過的な比較演算子を使用すべき", severity = "info"}
function rule:init(params) end
function rule:visit_template_specialization(spec)
    local template_name = spec:get_template_name()
    if template_name == "map" or template_name == "set" then
        local args = spec:get_template_arguments()
        if #args >= 2 and args[2]:get_name() == "std::less" then
            self:report_diagnostic({severity = self.severity, message = "std::less<void> (透過的比較演算子) の使用を検討してください", location = spec:get_location()})
        end
    end
end
return rule
