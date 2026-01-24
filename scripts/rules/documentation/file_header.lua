-- ファイルヘッダーコメントが必要
--
-- 説明:
--   ソースファイルにはファイルの説明を含むヘッダーコメントが必要です。
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度
--   - require_copyright: 著作権表示を必須とするか (デフォルト: false)
--   - min_lines: 最小行数 (デフォルト: 3)

local rule = {
    name = "file-header",
    description = "ファイルヘッダーコメントが必要",
    severity = "info",
}

function rule:init(params)
    self.require_copyright = params.require_copyright or false
    self.min_lines = params.min_lines or 3
end

function rule:check_file(file)
    local source = file:get_source_text()

    -- ファイル先頭のコメントをチェック
    local header_comment = source:match("^%s*/(%*.-*/)")

    if not header_comment then
        self:report_diagnostic({
            severity = self.severity,
            message = "ファイルヘッダーコメントがありません",
            location = file:create_location(1, 1),
            notes = {
                {
                    severity = "info",
                    message = "ファイルの説明、著作権、ライセンス情報などを含めてください",
                }
            }
        })
        return
    end

    -- コメントの行数をチェック
    local lines = 0
    for _ in header_comment:gmatch("\n") do
        lines = lines + 1
    end

    if lines < self.min_lines then
        self:report_diagnostic({
            severity = "info",
            message = string.format(
                "ファイルヘッダーコメントが短すぎます（%d行、最小%d行）",
                lines, self.min_lines
            ),
            location = file:create_location(1, 1),
        })
    end

    -- 著作権表示のチェック
    if self.require_copyright then
        if not header_comment:match("[Cc]opyright") and
           not header_comment:match("©") then
            self:report_diagnostic({
                severity = self.severity,
                message = "ファイルヘッダーに著作権表示がありません",
                location = file:create_location(1, 1),
            })
        end
    end
end

return rule
