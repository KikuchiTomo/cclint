-- 名前空間名はlowercaseであるべき
--
-- 説明:
--   名前空間名は小文字のみ、またはsnake_caseであるべきです。
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度
--   - allow_underscores: アンダースコアを許可 (デフォルト: true)

local rule = {
    name = "namespace-name-lowercase",
    description = "名前空間名はlowercaseであるべき",
    severity = "warning",
}

function rule:init(params)
    self.allow_underscores = params.allow_underscores ~= false
end

function rule:visit_namespace_decl(ns_decl)
    local name = ns_decl:get_name()

    -- 無名名前空間はスキップ
    if name == "" then
        return
    end

    -- lowercaseチェック
    local pattern = self.allow_underscores and "^[a-z][a-z0-9_]*$" or "^[a-z][a-z0-9]*$"

    if not name:match(pattern) then
        self:report_diagnostic({
            severity = self.severity,
            message = string.format(
                "名前空間名 '%s' は小文字であるべきです",
                name
            ),
            location = ns_decl:get_location(),
        })
    end
end

return rule
