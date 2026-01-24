-- ヘッダーファイルにはインクルードガードが必要
--
-- 説明:
--   ヘッダーファイルには#pragma onceまたは従来型のインクルードガードが必要です。
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度
--   - style: "pragma" または "ifndef" (デフォルト: "pragma")
--   - guard_pattern: ifndef形式のパターン (デフォルト: "{FILE}_{EXT}_")

local rule = {
    name = "header-guard",
    description = "ヘッダーファイルにはインクルードガードが必要",
    severity = "error",
}

function rule:init(params)
    self.style = params.style or "pragma"
    self.guard_pattern = params.guard_pattern or "{FILE}_{EXT}_"
end

function rule:check_file(file)
    local filename = file:get_filename()

    -- ヘッダーファイルのみチェック
    if not (filename:match("%.h$") or filename:match("%.hpp$") or
            filename:match("%.hh$") or filename:match("%.hxx$")) then
        return
    end

    local source = file:get_source_text()

    -- #pragma onceのチェック
    local has_pragma_once = source:match("#pragma%s+once")

    if self.style == "pragma" then
        if not has_pragma_once then
            self:report_diagnostic({
                severity = self.severity,
                message = "ヘッダーファイルに#pragma onceがありません",
                location = file:create_location(1, 1),
                notes = {
                    {
                        severity = "info",
                        message = "ファイル先頭に '#pragma once' を追加してください",
                    }
                }
            })
        end
    elseif self.style == "ifndef" then
        -- 従来型のインクルードガードをチェック
        local guard_name = self:generate_guard_name(filename)
        local pattern = string.format("#ifndef%s+%s", "%s", guard_name)

        if not source:match(pattern) then
            self:report_diagnostic({
                severity = self.severity,
                message = string.format(
                    "ヘッダーファイルに適切なインクルードガード（%s）がありません",
                    guard_name
                ),
                location = file:create_location(1, 1),
            })
        end
    end
end

-- インクルードガード名を生成
function rule:generate_guard_name(filename)
    local name = filename:match("([^/\\]+)$")  -- ファイル名のみ抽出
    name = name:gsub("%.", "_"):upper()  -- . を _ に置換して大文字化
    return name
end

return rule
