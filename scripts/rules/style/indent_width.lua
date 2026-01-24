-- インデント幅を統一
--
-- 説明:
--   インデント幅を統一することで、コードの可読性が向上します。
--   タブまたはスペースのいずれかに統一すべきです。
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度
--   - width: インデント幅（スペース数、デフォルト: 4）
--   - use_tabs: タブを使用するか (デフォルト: false)

local rule = {
    name = "indent-width",
    description = "インデント幅を統一",
    severity = "warning",
}

function rule:init(params)
    self.width = params.width or 4
    self.use_tabs = params.use_tabs or false
end

function rule:check_file(file)
    local source = file:get_source_text()
    local lines = {}
    for line in source:gmatch("([^\n]*)\n?") do
        table.insert(lines, line)
    end

    for line_num, line in ipairs(lines) do
        -- 空行はスキップ
        if line:match("^%s*$") then
            goto continue
        end

        -- 行頭の空白を取得
        local indent = line:match("^(%s+)")
        if not indent then
            goto continue
        end

        -- タブとスペースの混在をチェック
        local has_tabs = indent:match("\t") ~= nil
        local has_spaces = indent:match(" ") ~= nil

        if has_tabs and has_spaces then
            self:report_diagnostic({
                severity = self.severity,
                message = "インデントにタブとスペースが混在しています",
                location = file:create_location(line_num, 1),
            })
            goto continue
        end

        -- 期待されるインデント形式をチェック
        if self.use_tabs then
            if has_spaces then
                self:report_diagnostic({
                    severity = self.severity,
                    message = "インデントにはタブを使用すべきです",
                    location = file:create_location(line_num, 1),
                })
            end
        else
            if has_tabs then
                self:report_diagnostic({
                    severity = self.severity,
                    message = "インデントにはスペースを使用すべきです",
                    location = file:create_location(line_num, 1),
                })
            else
                -- スペースのインデント幅をチェック
                local spaces = #indent
                if spaces % self.width ~= 0 then
                    self:report_diagnostic({
                        severity = "info",
                        message = string.format(
                            "インデント幅が%dの倍数ではありません（現在: %d）",
                            self.width, spaces
                        ),
                        location = file:create_location(line_num, 1),
                    })
                end
            end
        end

        ::continue::
    end
end

return rule
