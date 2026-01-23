-- 宣言の後に空行を入れるべき
local rule = {name = "blank-line-after-declaration", description = "宣言の後に空行を入れるべき", severity = "info"}
function rule:init(params) end
function rule:check_file(file)
    -- 変数宣言の直後に実行文がある場合を検出
end
return rule
