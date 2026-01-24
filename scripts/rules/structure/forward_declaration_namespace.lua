-- 前方宣言は名前空間内で行うべき
--
-- 説明:
--   他の名前空間のクラスを前方宣言する場合、
--   適切な名前空間内で行うべきです。
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度

local rule = {
    name = "forward-declaration-namespace",
    description = "前方宣言は名前空間内で行うべき",
    severity = "warning",
}

function rule:init(params)
    -- パラメータなし
end

function rule:visit_class_decl(class_decl)
    -- 前方宣言のみチェック
    if class_decl:is_complete_definition() then
        return
    end

    -- 名前空間を取得
    local declared_ns = class_decl:get_namespace()

    -- 完全な定義を検索
    local definition = class_decl:get_definition()
    if not definition then
        return
    end

    local definition_ns = definition:get_namespace()

    -- 名前空間が一致しない場合
    if declared_ns ~= definition_ns then
        self:report_diagnostic({
            severity = self.severity,
            message = string.format(
                "前方宣言の名前空間（%s）が定義の名前空間（%s）と一致しません",
                declared_ns or "global", definition_ns or "global"
            ),
            location = class_decl:get_location(),
            notes = {
                {
                    severity = "info",
                    message = string.format(
                        "定義は %s にあります",
                        definition:get_location():to_string()
                    ),
                    location = definition:get_location(),
                }
            }
        })
    end
end

return rule
