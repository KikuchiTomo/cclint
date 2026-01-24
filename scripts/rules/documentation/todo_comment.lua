-- TODOコメントの形式チェック
--
-- 説明:
--   TODOコメントには担当者と説明を含めるべきです。
--   推奨形式: TODO(username): 説明
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度
--   - require_username: ユーザ名を必須とするか (デフォルト: true)
--   - allowed_tags: 許可するタグ (デフォルト: ["TODO", "FIXME", "XXX", "HACK"])

local rule = {
    name = "todo-comment",
    description = "TODOコメントの形式チェック",
    severity = "info",
}

function rule:init(params)
    self.require_username = params.require_username ~= false
    self.allowed_tags = params.allowed_tags or {"TODO", "FIXME", "XXX", "HACK"}
end

function rule:check_file(file)
    local source = file:get_source_text()
    local line_num = 1

    for line in source:gmatch("([^\n]*)\n?") do
        -- コメント行かチェック
        local comment = line:match("//%s*(.+)") or line:match("/%*%s*(.+)")

        if comment then
            for _, tag in ipairs(self.allowed_tags) do
                local tag_pos = comment:find(tag)

                if tag_pos then
                    -- TODO(username): 説明 形式をチェック
                    local has_username = comment:match(tag .. "%s*%(([^)]+)%)")
                    local has_colon = comment:match(tag .. ".*:")

                    if self.require_username and not has_username then
                        self:report_diagnostic({
                            severity = self.severity,
                            message = string.format(
                                "%sコメントにユーザ名が含まれていません",
                                tag
                            ),
                            location = file:create_location(line_num, tag_pos),
                            notes = {
                                {
                                    severity = "info",
                                    message = string.format(
                                        "推奨形式: %s(username): 説明",
                                        tag
                                    ),
                                }
                            }
                        })
                    elseif not has_colon then
                        self:report_diagnostic({
                            severity = "info",
                            message = string.format(
                                "%sコメントにコロン(:)が含まれていません",
                                tag
                            ),
                            location = file:create_location(line_num, tag_pos),
                        })
                    end

                    break  -- 1つのタグを見つけたら次の行へ
                end
            end
        end

        line_num = line_num + 1
    end
end

return rule
