-- 未初期化変数の使用を検出
local rule = {name = "uninitialized-variable", description = "未初期化変数の使用を検出", severity = "error"}
function rule:init(params) end
function rule:visit_var_decl(var_decl)
    if not var_decl:has_init() and not var_decl:is_static() then
        local var_type = var_decl:get_type()
        if var_type:is_builtin() and not var_type:is_pointer() then
            self:report_diagnostic({severity = self.severity, message = string.format("変数 '%s' が初期化されていません", var_decl:get_name()), location = var_decl:get_location()})
        end
    end
end
return rule
