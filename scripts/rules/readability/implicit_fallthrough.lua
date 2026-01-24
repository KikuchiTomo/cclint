-- switchのfallthrough意図を明示すべき
local rule = {name = "implicit-fallthrough", description = "switchのfallthrough意図を明示すべき", severity = "warning"}
function rule:init(params) end
function rule:visit_switch_case(case_stmt)
    if case_stmt:has_statements() and not case_stmt:ends_with_break() and not case_stmt:has_fallthrough_attribute() then
        self:report_diagnostic({severity = self.severity, message = "意図的なfallthroughの場合は[[fallthrough]]を使用してください", location = case_stmt:get_location()})
    end
end
return rule
