-- 関数の長さを制限
--
-- 説明:
--   長すぎる関数は理解が困難で保守性が低下します。
--   関数は適切な長さに保つべきです。
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度
--   - max_lines: 最大行数 (デフォルト: 50)
--   - count_comments: コメント行もカウントするか (デフォルト: false)
--   - count_empty_lines: 空行もカウントするか (デフォルト: false)

local rule = {
    name = "max-function-length",
    description = "関数の長さを制限",
    severity = "warning",
}

function rule:init(params)
    self.max_lines = params.max_lines or 50
    self.count_comments = params.count_comments or false
    self.count_empty_lines = params.count_empty_lines or false
end

function rule:visit_function_decl(func_decl)
    if not func_decl:has_body() then
        return
    end

    local body = func_decl:get_body()
    local start_line = body:get_begin_location():get_line()
    local end_line = body:get_end_location():get_line()
    local total_lines = end_line - start_line + 1

    -- 実効行数を計算
    local effective_lines = total_lines
    if not self.count_comments or not self.count_empty_lines then
        effective_lines = self:count_effective_lines(body, start_line, end_line)
    end

    if effective_lines > self.max_lines then
        self:report_diagnostic({
            severity = self.severity,
            message = string.format(
                "関数 '%s' が長すぎます（%d行、最大%d行）",
                func_decl:get_name(), effective_lines, self.max_lines
            ),
            location = func_decl:get_location(),
            notes = {
                {
                    severity = "info",
                    message = "関数を複数の小さな関数に分割することを検討してください",
                }
            }
        })
    end
end

-- 実効行数をカウント
function rule:count_effective_lines(body, start_line, end_line)
    local source = body:get_source_text()
    local lines = {}
    for line in source:gmatch("([^\n]*)\n?") do
        table.insert(lines, line)
    end

    local count = 0
    for _, line in ipairs(lines) do
        local trimmed = line:match("^%s*(.-)%s*$")

        -- 空行のチェック
        if trimmed == "" then
            if self.count_empty_lines then
                count = count + 1
            end
            goto continue
        end

        -- コメント行のチェック
        if trimmed:match("^//") or trimmed:match("^/%*") or trimmed:match("^%*") then
            if self.count_comments then
                count = count + 1
            end
            goto continue
        end

        -- 実効行としてカウント
        count = count + 1

        ::continue::
    end

    return count
end

return rule
