-- ポインタ宣言のスタイルを統一
--
-- 説明:
--   ポインタ宣言のアスタリスク位置を統一すべきです。
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度
--   - style: "left", "right", "middle" (デフォルト: "left")

local rule = {
    name = "consistent-pointer-declaration",
    description = "ポインタ宣言のスタイルを統一",
    severity = "info",
}

function rule:init(params)
    self.style = params.style or "left"
end

function rule:check_file(file)
    local source = file:get_source_text()
    local line_num = 1

    for line in source:gmatch("([^\n]*)\n?") do
        -- ポインタ宣言のパターンをチェック
        if self.style == "left" then
            -- int* p が正しい、int *p や int * p は警告
            if line:match("[%w_]+%s+%*%s*[%w_]+") and 
               not line:match("[%w_]+%*%s*[%w_]+") then
                self:report_diagnostic({
                    severity = self.severity,
                    message = "ポインタのアスタリスクは型に付けるべきです（int* p）",
                    location = file:create_location(line_num, 1),
                })
            end
        end

        line_num = line_num + 1
    end
end

return rule
