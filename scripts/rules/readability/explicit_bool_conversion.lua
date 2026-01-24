-- bool変換は明示的に行うべき
local rule = {name = "explicit-bool-conversion", description = "bool変換は明示的に行うべき", severity = "info"}
function rule:init(params) end
function rule:visit_implicit_cast_expr(cast)
    if cast:get_cast_kind() == "IntegralToBoolean" or cast:get_cast_kind() == "PointerToBoolean" then
        self:report_diagnostic({severity = self.severity, message = "bool変換は明示的に行うべきです", location = cast:get_location()})
    end
end
return rule
