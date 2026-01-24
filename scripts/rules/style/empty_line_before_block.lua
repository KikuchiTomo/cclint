-- ブロックの前に空行を入れるべき
local rule = {name = "empty-line-before-block", description = "ブロックの前に空行を入れるべき", severity = "info"}
function rule:init(params) end
function rule:check_file(file)
    -- if/for/whileブロックの前に空行があるかチェック
end
return rule
