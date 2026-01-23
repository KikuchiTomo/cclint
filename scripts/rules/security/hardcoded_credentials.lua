-- ハードコードされた認証情報を検出
--
-- 説明:
--   パスワードやAPIキーなどの認証情報がハードコードされていないかチェックします。
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度

local rule = {
    name = "hardcoded-credentials",
    description = "ハードコードされた認証情報を検出",
    severity = "error",
}

function rule:init(params)
    -- パラメータなし
end

function rule:visit_var_decl(var_decl)
    local name = var_decl:get_name():lower()
    
    -- 疑わしい変数名
    local suspicious_names = {
        "password", "passwd", "pwd",
        "api_key", "apikey", "secret",
        "token", "auth", "credential"
    }

    for _, suspicious in ipairs(suspicious_names) do
        if name:match(suspicious) then
            -- 文字列リテラルで初期化されているかチェック
            if var_decl:has_init() then
                local init = var_decl:get_init()
                if init:is_string_literal() then
                    self:report_diagnostic({
                        severity = self.severity,
                        message = string.format(
                            "認証情報がハードコードされている可能性があります（%s）",
                            var_decl:get_name()
                        ),
                        location = var_decl:get_location(),
                        notes = {
                            {
                                severity = "info",
                                message = "環境変数や設定ファイルから読み込むべきです",
                            }
                        }
                    })
                end
            end
            break
        end
    end
end

return rule
