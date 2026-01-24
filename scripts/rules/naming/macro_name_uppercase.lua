-- マクロ名はUPPER_CASEであるべき
local rule = {name = "macro-name-uppercase", description = "マクロ名はUPPER_CASEであるべき", severity = "warning"}
function rule:init(params) end
function rule:visit_macro_definition(macro_def)
    local name = macro_def:get_name()
    if not name:match("^[A-Z][A-Z0-9_]*$") and not name:match("^__") then
        self:report_diagnostic({severity = self.severity, message = string.format("マクロ名 '%s' はUPPER_CASEであるべきです", name), location = macro_def:get_location()})
    end
end
return rule
