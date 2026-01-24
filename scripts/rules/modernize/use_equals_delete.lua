-- 禁止すべき関数は= deleteを使用すべき
--
-- 説明:
--   コピーコンストラクタなどを禁止する場合、
--   privateにするのではなく= deleteを使用すべきです。
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度

local rule = {
    name = "use-equals-delete",
    description = "禁止すべき関数は= deleteを使用すべき",
    severity = "info",
}

function rule:init(params)
    -- パラメータなし
end

function rule:visit_class_decl(class_decl)
    -- private領域の宣言のみの特殊メンバ関数をチェック
    for _, method in ipairs(class_decl:get_methods()) do
        -- 特殊メンバ関数のみ
        if not (method:is_copy_constructor() or method:is_copy_assignment()) then
            goto continue
        end

        -- privateかつ宣言のみ（定義なし）
        if method:get_access() == "private" and not method:has_body() then
            self:report_diagnostic({
                severity = self.severity,
                message = string.format(
                    "%sを禁止する場合は= deleteを使用すべきです",
                    method:is_copy_constructor() and "コピーコンストラクタ" or "コピー代入演算子"
                ),
                location = method:get_location(),
                notes = {
                    {
                        severity = "info",
                        message = "publicに移動して= deleteを追加してください",
                    }
                }
            })
        end

        ::continue::
    end
end

return rule
