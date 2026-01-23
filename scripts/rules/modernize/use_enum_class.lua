-- enum classを使用すべき
local rule = {name = "use-enum-class", description = "enum classを使用すべき", severity = "info"}
function rule:init(params) end
function rule:visit_enum_decl(enum_decl)
    if not enum_decl:is_scoped() then
        self:report_diagnostic({severity = self.severity, message = string.format("enum '%s' はenum classにすべきです", enum_decl:get_name()), location = enum_decl:get_location(), notes = {{severity = "info", message = "型安全性とスコープの明確化のためenum classを使用してください"}}})
    end
end
return rule
