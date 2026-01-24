-- 一貫した波括弧の使用
local rule = {name = "consistent-bracing", description = "一貫した波括弧の使用", severity = "info"}
function rule:init(params) self.require_braces = params.require_braces ~= false end
function rule:visit_if_stmt(if_stmt)
    if self.require_braces then
        local then_branch = if_stmt:get_then()
        if not then_branch:is_compound_stmt() then
            self:report_diagnostic({severity = self.severity, message = "if文には常に波括弧を使用すべきです", location = if_stmt:get_location()})
        end
    end
end
return rule
