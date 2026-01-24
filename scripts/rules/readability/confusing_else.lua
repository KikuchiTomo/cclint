-- 混乱しやすいelse文を警告
local rule = {name = "confusing-else", description = "混乱しやすいelse文を警告", severity = "info"}
function rule:init(params) end
function rule:visit_if_stmt(if_stmt)
    local else_branch = if_stmt:get_else()
    if else_branch and else_branch:is_if_stmt() then
        self:report_diagnostic({severity = self.severity, message = "else ifではなくif-else ifチェーンを明確にすべきです", location = else_branch:get_location(), notes = {{severity = "info", message = "else if (...) の形式を使用してください"}}})
    end
end
return rule
