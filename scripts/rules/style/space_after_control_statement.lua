-- 制御文の後にスペースを入れるべき
--
-- 説明:
--   if, for, whileなどの制御文の後には
--   スペースを1つ入れるべきです。
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度

local rule = {
    name = "space-after-control-statement",
    description = "制御文の後にスペースを入れるべき",
    severity = "info",
}

function rule:init(params)
    -- パラメータなし
end

function rule:check_file(file)
    local source = file:get_source_text()
    local control_keywords = {"if", "for", "while", "switch"}

    local line_num = 1
    for line in source:gmatch("([^\n]*)\n?") do
        for _, keyword in ipairs(control_keywords) do
            -- keyword( のパターンを検出
            local pattern = keyword .. "%s*%("
            local pos = 1

            while pos <= #line do
                local start_pos, end_pos = line:find(keyword .. "%(", pos)

                if not start_pos then
                    break
                end

                -- キーワードと括弧の間にスペースがない
                self:report_diagnostic({
                    severity = self.severity,
                    message = string.format(
                        "'%s'の後にスペースを入れるべきです",
                        keyword
                    ),
                    location = file:create_location(line_num, start_pos),
                })

                pos = end_pos + 1
            end
        end

        line_num = line_num + 1
    end
end

return rule
