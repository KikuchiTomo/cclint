-- 構造体初期化にdesignated initializersを使用すべき
local rule = {name = "use-designated-initializers", description = "構造体初期化にdesignated initializersを使用すべき", severity = "info"}
function rule:init(params) end
function rule:visit_init_list_expr(init_list)
    if init_list:initializes_struct() and not init_list:uses_designated_initializers() then
        self:report_diagnostic({severity = self.severity, message = "designated initializers (.field = value) の使用を検討してください", location = init_list:get_location()})
    end
end
return rule
