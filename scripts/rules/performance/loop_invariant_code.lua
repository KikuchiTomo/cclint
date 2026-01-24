-- ループ不変なコードはループ外に移動すべき
local rule = {name = "loop-invariant-code", description = "ループ不変なコードはループ外に移動すべき", severity = "info"}
function rule:init(params) end
function rule:visit_for_stmt(for_stmt)
    -- 簡易実装：関数呼び出しで引数が全て不変な場合を検出
end
return rule
