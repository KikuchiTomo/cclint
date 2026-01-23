-- エスケープが多い文字列はraw string literalを使用すべき
--
-- 説明:
--   多数のエスケープシーケンスを含む文字列は、
--   raw string literal (R"()")を使用すべきです。
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度
--   - min_escapes: raw stringを推奨する最小エスケープ数 (デフォルト: 3)

local rule = {
    name = "raw-string-literal",
    description = "エスケープが多い文字列はraw string literalを使用すべき",
    severity = "info",
}

function rule:init(params)
    self.min_escapes = params.min_escapes or 3
end

function rule:visit_string_literal(literal)
    local value = literal:get_value()
    
    -- エスケープシーケンスをカウント
    local escape_count = 0
    for _ in value:gmatch("\\") do
        escape_count = escape_count + 1
    end

    if escape_count >= self.min_escapes then
        self:report_diagnostic({
            severity = self.severity,
            message = string.format(
                "文字列に%dつのエスケープがあります",
                escape_count
            ),
            location = literal:get_location(),
            notes = {
                {
                    severity = "info",
                    message = "raw string literal (R\"()\" ) の使用を検討してください",
                }
            }
        })
    end
end

return rule
