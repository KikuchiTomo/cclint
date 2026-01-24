-- TOCTOU競合状態の可能性を検出
local rule = {name = "toctou-race", description = "TOCTOU競合状態の可能性を検出", severity = "warning"}
function rule:init(params) end
function rule:visit_function_decl(func_decl)
    -- ファイル存在チェック後のファイル操作パターンを検出
end
return rule
