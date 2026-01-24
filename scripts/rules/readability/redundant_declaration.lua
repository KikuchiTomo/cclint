-- 冗長な宣言を削除すべき
local rule = {name = "redundant-declaration", description = "冗長な宣言を削除すべき", severity = "info"}
function rule:init(params) end
function rule:visit_function_decl(func_decl)
    if func_decl:has_body() and func_decl:has_previous_declaration() then
        local prev = func_decl:get_previous_declaration()
        if prev:get_file() == func_decl:get_file() then
            self:report_diagnostic({severity = self.severity, message = "同じファイル内の前方宣言は不要です", location = prev:get_location()})
        end
    end
end
return rule
