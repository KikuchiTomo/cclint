-- 仮想関数のデフォルト引数を避けるべき
local rule = {name = "avoid-default-arguments-virtual", description = "仮想関数のデフォルト引数を避けるべき", severity = "warning"}
function rule:init(params) end
function rule:visit_method_decl(method_decl)
    if method_decl:is_virtual() and method_decl:has_default_arguments() then
        self:report_diagnostic({severity = self.severity, message = "仮想関数にはデフォルト引数を使用すべきではありません", location = method_decl:get_location(), notes = {{severity = "info", message = "オーバーライドされた関数では基底クラスのデフォルト値が使用されません"}}})
    end
end
return rule
