-- 範囲forループを使用すべき
local rule = {name = "use-range-based-for", description = "範囲forループを使用すべき", severity = "info"}
function rule:init(params) end
function rule:visit_for_stmt(for_stmt)
    -- イテレータを使った従来型forループを検出
end
return rule
