-- C言語スタイルのキャストを避けるべき
local rule = {name = "avoid-c-cast", description = "C言語スタイルのキャストを避けるべき", severity = "warning"}
function rule:init(params) end
function rule:visit_c_style_cast_expr(cast)
    local cast_type = cast:get_type():get_name()
    self:report_diagnostic({severity = self.severity, message = "C言語スタイルのキャストは避けるべきです", location = cast:get_location(), notes = {{severity = "info", message = "static_cast, dynamic_cast, const_cast, reinterpret_castを使用してください"}}})
end
return rule
