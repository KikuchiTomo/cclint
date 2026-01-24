-- 冗長な文字列初期化を避けるべき
local rule = {name = "redundant-string-init", description = "冗長な文字列初期化を避けるべき", severity = "info"}
function rule:init(params) end
function rule:visit_var_decl(var_decl)
    if var_decl:get_type():is_string() and var_decl:has_init() then
        local init = var_decl:get_init()
        if init:is_string_literal() and init:get_value() == "" then
            self:report_diagnostic({severity = self.severity, message = "std::stringのデフォルトコンストラクタは空文字列を生成します", location = var_decl:get_location()})
        end
    end
end
return rule
