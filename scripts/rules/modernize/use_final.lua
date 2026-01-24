-- 継承を禁止するクラスにはfinalを付けるべき
local rule = {name = "use-final", description = "継承を禁止するクラスにはfinalを付けるべき", severity = "info"}
function rule:init(params) end
function rule:visit_class_decl(class_decl)
    if class_decl:has_virtual_destructor() and not class_decl:is_final() and not class_decl:has_derived_classes() then
        self:report_diagnostic({severity = self.severity, message = string.format("クラス '%s' はfinal指定を検討できます", class_decl:get_name()), location = class_decl:get_location()})
    end
end
return rule
