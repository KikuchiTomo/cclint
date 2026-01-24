-- 冗長なboolean式を簡略化すべき
--
-- 説明:
--   冗長なboolean式（if (x == true)など）を簡略化すべきです。
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度

local rule = {
    name = "simplify-boolean-expr",
    description = "冗長なboolean式を簡略化すべき",
    severity = "info",
}

function rule:init(params)
    -- パラメータなし
end

function rule:visit_binary_operator(bin_op)
    local op = bin_op:get_operator()

    -- == や != 演算子のみチェック
    if op ~= "==" and op ~= "!=" then
        return
    end

    local lhs = bin_op:get_lhs()
    local rhs = bin_op:get_rhs()

    -- 片方がbooleanリテラル (true/false) かチェック
    local bool_literal = nil
    local other_expr = nil

    if lhs:is_boolean_literal() then
        bool_literal = lhs
        other_expr = rhs
    elseif rhs:is_boolean_literal() then
        bool_literal = rhs
        other_expr = lhs
    else
        return
    end

    local bool_value = bool_literal:get_value()
    local simplified = nil

    -- 簡略化された式を生成
    if op == "==" then
        if bool_value then
            simplified = other_expr:get_source_text()  -- x == true -> x
        else
            simplified = "!" .. other_expr:get_source_text()  -- x == false -> !x
        end
    elseif op == "!=" then
        if bool_value then
            simplified = "!" .. other_expr:get_source_text()  -- x != true -> !x
        else
            simplified = other_expr:get_source_text()  -- x != false -> x
        end
    end

    if simplified then
        self:report_diagnostic({
            severity = self.severity,
            message = "冗長なboolean式を簡略化できます",
            location = bin_op:get_location(),
            fix_hints = {
                {
                    range = bin_op:get_range(),
                    replacement = simplified,
                }
            }
        })
    end
end

return rule
