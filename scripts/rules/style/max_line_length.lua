-- 1行の最大文字数を制限
--
-- 説明:
--   長すぎる行は可読性を低下させます。
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度
--   - max_length: 最大文字数 (デフォルト: 100)

local rule = {
    name = "max-line-length",
    description = "1行の最大文字数を制限",
    severity = "warning",
}

function rule:init(params)
    self.max_length = params.max_length or 100
end

function rule:check_file(file)
    local source = file:get_source_text()
    local line_num = 1

    for line in source:gmatch("([^\n]*)\n?") do
        local length = #line

        if length > self.max_length then
            self:report_diagnostic({
                severity = self.severity,
                message = string.format(
                    "行が長すぎます（%d文字、最大%d文字）",
                    length, self.max_length
                ),
                location = file:create_location(line_num, self.max_length + 1),
            })
        end

        line_num = line_num + 1
    end
end

return rule
