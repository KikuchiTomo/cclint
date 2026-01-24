-- タブ文字を使用すべきでない
--
-- 説明:
--   タブ文字の代わりにスペースを使用すべきです。
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度

local rule = {
    name = "no-tab-character",
    description = "タブ文字を使用すべきでない",
    severity = "info",
}

function rule:init(params)
    -- パラメータなし
end

function rule:check_file(file)
    local source = file:get_source_text()
    local line_num = 1

    for line in source:gmatch("([^\n]*)\n?") do
        if line:match("\t") then
            local col = line:find("\t")
            self:report_diagnostic({
                severity = self.severity,
                message = "タブ文字が使用されています",
                location = file:create_location(line_num, col),
                notes = {
                    {
                        severity = "info",
                        message = "スペースを使用してください",
                    }
                }
            })
        end

        line_num = line_num + 1
    end
end

return rule
