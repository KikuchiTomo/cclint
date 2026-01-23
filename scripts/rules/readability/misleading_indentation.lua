-- 誤解を招くインデントを検出
local rule = {name = "misleading-indentation", description = "誤解を招くインデントを検出", severity = "warning"}
function rule:init(params) end
function rule:visit_if_stmt(if_stmt)
    local then_branch = if_stmt:get_then()
    if not then_branch:is_compound_stmt() then
        -- 次の文のインデントをチェック
    end
end
return rule
