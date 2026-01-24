-- 行末の空白を削除すべき
--
-- 説明:
--   行末の空白は不要であり、削除すべきです。
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度

local rule = {
    name = "trailing-whitespace",
    description = "行末の空白を削除すべき",
    severity = "info",
}

function rule:init(params)
    -- パラメータなし
end

function rule:check_file(file)
    local source = file:get_source_text()
    local line_num = 1

    for line in source:gmatch("([^\n]*)\n?") do
        -- 行末に空白があるかチェック
        if line:match("%s+$") then
            local trailing_start = #line
            while trailing_start > 0 and line:sub(trailing_start, trailing_start):match("%s") do
                trailing_start = trailing_start - 1
            end

            self:report_diagnostic({
                severity = self.severity,
                message = "行末に空白があります",
                location = file:create_location(line_num, trailing_start + 1),
            })
        end

        line_num = line_num + 1
    end
end

return rule
