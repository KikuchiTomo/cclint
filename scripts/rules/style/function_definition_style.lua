-- 関数定義のスタイルを統一
local rule = {name = "function-definition-style", description = "関数定義のスタイルを統一", severity = "info"}
function rule:init(params) self.return_type_on_own_line = params.return_type_on_own_line or false end
function rule:visit_function_decl(func_decl)
    if not func_decl:has_body() then return end
    -- 戻り値型が独立した行にあるかチェック
end
return rule
