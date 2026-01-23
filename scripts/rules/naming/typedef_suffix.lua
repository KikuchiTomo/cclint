-- typedef型には_tサフィックスを付けるべき
--
-- 説明:
--   typedef型には_tサフィックスを付けると、
--   型であることが明確になります。
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度
--   - suffix: サフィックス (デフォルト: "_t")

local rule = {
    name = "typedef-suffix",
    description = "typedef型には_tサフィックスを付けるべき",
    severity = "info",
}

function rule:init(params)
    self.suffix = params.suffix or "_t"
end

function rule:visit_typedef_decl(typedef_decl)
    local name = typedef_decl:get_name()

    if not name:match(self.suffix .. "$") then
        self:report_diagnostic({
            severity = self.severity,
            message = string.format(
                "typedef型 '%s' には '%s' サフィックスを付けるべきです",
                name, self.suffix
            ),
            location = typedef_decl:get_location(),
        })
    end
end

return rule
