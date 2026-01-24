-- 複数行コメントのスタイルを統一
local rule = {name = "multiline-comment-style", description = "複数行コメントのスタイルを統一", severity = "info"}
function rule:init(params) self.style = params.style or "starred" end
function rule:check_file(file)
    -- /* */ スタイルのコメントをチェック
end
return rule
