-- 構造化束縛を使用すべき（C++17）
local rule = {name = "use-structured-bindings", description = "構造化束縛を使用すべき（C++17）", severity = "info"}
function rule:init(params) end
function rule:visit_for_stmt(for_stmt)
    -- pair/tupleを返すイテレーションで構造化束縛が使えるケースを検出
end
return rule
