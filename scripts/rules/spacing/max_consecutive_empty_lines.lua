-- 連続する空行の最大数を制限
--
-- 説明:
--   連続する空行が多すぎるとコードが読みにくくなります。
--   デフォルトでは2行以上の連続する空行を警告します。
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度
--   - max_lines: 許可する最大連続空行数 (デフォルト: 1)

local rule = {
    name = "max-consecutive-empty-lines",
    description = "連続する空行の最大数を制限",
    severity = "warning",
}

function rule:init(params)
    self.max_lines = params.max_lines or 1
end

function rule:check_file(file)
    local source = file:get_source_text()
    local lines = {}
    for line in source:gmatch("([^\n]*)\n?") do
        table.insert(lines, line)
    end

    local consecutive_empty = 0
    local empty_start_line = nil

    for i, line in ipairs(lines) do
        -- 空行かチェック（空白文字のみの行も空行とみなす）
        if line:match("^%s*$") then
            if consecutive_empty == 0 then
                empty_start_line = i
            end
            consecutive_empty = consecutive_empty + 1

            -- 連続空行が閾値を超えたら警告
            if consecutive_empty > self.max_lines then
                self:report_diagnostic({
                    severity = self.severity,
                    message = string.format(
                        "%d行の連続する空行があります（最大%d行まで）",
                        consecutive_empty, self.max_lines
                    ),
                    location = file:create_location(i, 1),
                })
            end
        else
            consecutive_empty = 0
            empty_start_line = nil
        end
    end
end

return rule
