-- 列挙型名はCamelCaseであるべき
--
-- 説明:
--   列挙型（enum）名はCamelCase形式であるべきです。
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度

local rule = {
    name = "enum-name-camelcase",
    description = "列挙型名はCamelCaseであるべき",
    severity = "warning",
}

function rule:init(params)
    -- パラメータなし
end

function rule:visit_enum_decl(enum_decl)
    local name = enum_decl:get_name()

    -- 無名列挙型はスキップ
    if name == "" then
        return
    end

    -- CamelCaseチェック
    if not name:match("^[A-Z][a-zA-Z0-9]*$") then
        self:report_diagnostic({
            severity = self.severity,
            message = string.format(
                "列挙型名 '%s' はCamelCaseであるべきです",
                name
            ),
            location = enum_decl:get_location(),
        })
    end
end

return rule
