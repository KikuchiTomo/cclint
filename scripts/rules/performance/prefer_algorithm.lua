-- <algorithm>の関数を使用すべき
local rule = {name = "prefer-algorithm", description = "<algorithm>の関数を使用すべき", severity = "info"}
function rule:init(params) end
function rule:visit_for_stmt(for_stmt)
    -- 簡単なループパターンを検出してstd::algorithmを提案
end
return rule
