-- constexprを使用すべき
--
-- 説明:
--   コンパイル時定数にはconstexprを使用すべきです。
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度

local rule = {
    name = "use-constexpr",
    description = "constexprを使用すべき",
    severity = "info",
}

function rule:init(params)
    -- パラメータなし
end

function rule:visit_var_decl(var_decl)
    -- const変数のみチェック
    if not var_decl:is_const() then
        return
    end

    -- すでにconstexprの場合はスキップ
    if var_decl:is_constexpr() then
        return
    end

    -- 初期化式があるかチェック
    if not var_decl:has_init() then
        return
    end

    local init_expr = var_decl:get_init()

    -- コンパイル時定数式かチェック
    if init_expr:is_constant_expression() then
        self:report_diagnostic({
            severity = self.severity,
            message = string.format(
                "変数 '%s' はconstexprにできます",
                var_decl:get_name()
            ),
            location = var_decl:get_location(),
            fix_hints = {
                {
                    range = var_decl:get_type_range(),
                    replacement = "constexpr",
                }
            }
        })
    end
end

return rule
