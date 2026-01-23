-- typedefの代わりにusingを使用すべき
--
-- 説明:
--   C++11以降、typedefよりもusing（型エイリアス）を使用すべきです。
--   より読みやすく、テンプレートエイリアスもサポートします。
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度

local rule = {
    name = "use-using",
    description = "typedefの代わりにusingを使用すべき",
    severity = "info",
}

function rule:init(params)
    -- パラメータなし
end

function rule:visit_typedef_decl(typedef_decl)
    local name = typedef_decl:get_name()
    local underlying_type = typedef_decl:get_underlying_type()

    self:report_diagnostic({
        severity = self.severity,
        message = string.format(
            "typedefの代わりにusingを使用すべきです（typedef %s）",
            name
        ),
        location = typedef_decl:get_location(),
        fix_hints = {
            {
                range = typedef_decl:get_range(),
                replacement = string.format(
                    "using %s = %s",
                    name, underlying_type:get_name()
                ),
            }
        }
    })
end

return rule
