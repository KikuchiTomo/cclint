-- グローバル変数には接頭辞を付けるべき
local rule = {name = "global-variable-prefix", description = "グローバル変数には接頭辞を付けるべき", severity = "warning"}
function rule:init(params) self.prefix = params.prefix or "g_" end
function rule:visit_var_decl(var_decl)
    if var_decl:has_global_storage() and not var_decl:is_const() then
        local name = var_decl:get_name()
        if not name:match("^" .. self.prefix) then
            self:report_diagnostic({severity = self.severity, message = string.format("グローバル変数 '%s' には接頭辞 '%s' を付けるべきです", name, self.prefix), location = var_decl:get_location()})
        end
    end
end
return rule
