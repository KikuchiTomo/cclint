-- ファンクタよりラムダを使用すべき
local rule = {name = "use-lambda", description = "ファンクタよりラムダを使用すべき", severity = "info"}
function rule:init(params) end
function rule:visit_class_decl(class_decl)
    if class_decl:has_operator_call() and class_decl:get_methods_count() == 1 then
        self:report_diagnostic({severity = self.severity, message = string.format("クラス '%s' はラムダに置き換え可能です", class_decl:get_name()), location = class_decl:get_location()})
    end
end
return rule
