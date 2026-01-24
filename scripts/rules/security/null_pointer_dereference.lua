-- null ポインタデリファレンスの可能性を検出
local rule = {name = "null-pointer-dereference", description = "nullポインタデリファレンスの可能性を検出", severity = "error"}
function rule:init(params) end
function rule:visit_member_expr(member_expr)
    local base = member_expr:get_base()
    if base:is_decl_ref() and base:get_type():is_pointer() then
        -- ポインタのnullチェックがあるかを確認
    end
end
return rule
