-- デフォルト実装は= defaultを使用すべき
--
-- 説明:
--   コンストラクタ/デストラクタがデフォルト実装の場合、
--   = defaultを使用すべきです。
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度

local rule = {
    name = "use-equals-default",
    description = "デフォルト実装は= defaultを使用すべき",
    severity = "info",
}

function rule:init(params)
    -- パラメータなし
end

function rule:visit_method_decl(method_decl)
    -- コンストラクタまたはデストラクタのみ
    if not (method_decl:is_constructor() or method_decl:is_destructor() or
            method_decl:is_copy_constructor() or method_decl:is_move_constructor() or
            method_decl:is_copy_assignment() or method_decl:is_move_assignment()) then
        return
    end

    -- すでに= defaultの場合はスキップ
    if method_decl:is_defaulted() then
        return
    end

    -- 宣言のみの場合はスキップ
    if not method_decl:has_body() then
        return
    end

    -- 本体が空かチェック
    local body = method_decl:get_body()
    local stmts = body:get_statements()

    if #stmts == 0 or (#stmts == 1 and stmts[1]:is_null_statement()) then
        self:report_diagnostic({
            severity = self.severity,
            message = "空の実装は= defaultを使用すべきです",
            location = method_decl:get_location(),
            fix_hints = {
                {
                    range = method_decl:get_body_range(),
                    replacement = "= default;",
                }
            }
        })
    end
end

return rule
