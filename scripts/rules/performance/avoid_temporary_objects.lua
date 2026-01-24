-- 一時オブジェクトの生成を避けるべき
local rule = {name = "avoid-temporary-objects", description = "一時オブジェクトの生成を避けるべき", severity = "info"}
function rule:init(params) end
function rule:visit_construct_expr(construct_expr)
    if construct_expr:is_temporary() and construct_expr:get_parent():is_call_expr() then
        self:report_diagnostic({severity = self.severity, message = "一時オブジェクトの生成を避けるべきです", location = construct_expr:get_location(), notes = {{severity = "info", message = "参照渡しまたは移動セマンティクスの使用を検討してください"}}})
    end
end
return rule
