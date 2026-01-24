-- 演算子の前後にスペースを入れるべき
--
-- 説明:
--   演算子の前後には適切なスペースを入れるべきです。
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度

local rule = {
    name = "operator-spacing",
    description = "演算子の前後にスペースを入れるべき",
    severity = "info",
}

function rule:init(params)
    -- パラメータなし
end

function rule:check_file(file)
    local source = file:get_source_text()
    local operators = {"+", "-", "*", "/", "=", "==", "!=", "<", ">", "<=", ">="}

    local line_num = 1
    for line in source:gmatch("([^\n]*)\n?") do
        for _, op in ipairs(operators) do
            -- 演算子の前後にスペースがないパターンを検出
            -- ただし、++, --, ->, :: などの特殊な演算子は除外

            -- [非空白][演算子][非空白] のパターン
            local pattern = "[^%s%-+]" .. op:gsub("([%-%+%*])", "%%%1") .. "[^%s%->]"

            if line:match(pattern) then
                self:report_diagnostic({
                    severity = self.severity,
                    message = string.format(
                        "演算子 '%s' の前後にスペースを入れるべきです",
                        op
                    ),
                    location = file:create_location(line_num, 1),
                })
            end
        end

        line_num = line_num + 1
    end
end

return rule
