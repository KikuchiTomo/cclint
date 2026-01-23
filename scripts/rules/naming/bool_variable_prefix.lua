-- bool変数にはis/has接頭辞を付けるべき
local rule = {name = "bool-variable-prefix", description = "bool変数にはis/has接頭辞を付けるべき", severity = "info"}
function rule:init(params) end
function rule:visit_var_decl(var_decl)
    if var_decl:get_type():is_bool() then
        local name = var_decl:get_name()
        if not (name:match("^is") or name:match("^has") or name:match("^should") or name:match("^can")) then
            self:report_diagnostic({severity = self.severity, message = string.format("bool変数 '%s' にはis/has/should/can接頭辞を付けるべきです", name), location = var_decl:get_location()})
        end
    end
end
return rule
