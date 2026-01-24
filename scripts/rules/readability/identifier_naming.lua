-- 識別子の長さを適切に保つべき
--
-- 説明:
--   識別子が短すぎたり長すぎたりすると可読性が低下します。
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度
--   - min_length: 最小長 (デフォルト: 2)
--   - max_length: 最大長 (デフォルト: 30)
--   - ignored_names: 無視する名前のリスト (デフォルト: ["i", "j", "k", "x", "y", "z"])

local rule = {
    name = "identifier-naming",
    description = "識別子の長さを適切に保つべき",
    severity = "info",
}

function rule:init(params)
    self.min_length = params.min_length or 2
    self.max_length = params.max_length or 30
    self.ignored_names = params.ignored_names or {"i", "j", "k", "x", "y", "z"}
end

function rule:visit_var_decl(var_decl)
    self:check_identifier_length(var_decl:get_name(), var_decl:get_location())
end

function rule:visit_function_decl(func_decl)
    self:check_identifier_length(func_decl:get_name(), func_decl:get_location())
end

function rule:check_identifier_length(name, location)
    if name == "" then
        return
    end

    -- 無視リストチェック
    for _, ignored in ipairs(self.ignored_names) do
        if name == ignored then
            return
        end
    end

    local length = #name

    if length < self.min_length then
        self:report_diagnostic({
            severity = self.severity,
            message = string.format(
                "識別子 '%s' が短すぎます（%d文字、最小%d文字）",
                name, length, self.min_length
            ),
            location = location,
        })
    elseif length > self.max_length then
        self:report_diagnostic({
            severity = self.severity,
            message = string.format(
                "識別子 '%s' が長すぎます（%d文字、最大%d文字）",
                name, length, self.max_length
            ),
            location = location,
        })
    end
end

return rule
