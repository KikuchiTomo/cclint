-- テンプレートパラメータ名はCamelCaseであるべき
local rule = {name = "template-parameter-name", description = "テンプレートパラメータ名はCamelCaseであるべき", severity = "info"}
function rule:init(params) end
function rule:visit_template_type_parm(param)
    local name = param:get_name()
    if name ~= "" and not name:match("^[A-Z][a-zA-Z0-9]*$") then
        self:report_diagnostic({severity = self.severity, message = string.format("テンプレートパラメータ '%s' はCamelCaseであるべきです", name), location = param:get_location()})
    end
end
return rule
