-- 関数の認知的複雑度を制限
--
-- 説明:
--   関数の認知的複雑度が高すぎると理解が困難になります。
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度
--   - max_complexity: 最大複雑度 (デフォルト: 15)

local rule = {
    name = "function-cognitive-complexity",
    description = "関数の認知的複雑度を制限",
    severity = "warning",
}

function rule:init(params)
    self.max_complexity = params.max_complexity or 15
end

function rule:visit_function_decl(func_decl)
    if not func_decl:has_body() then
        return
    end

    local complexity = self:calculate_complexity(func_decl:get_body())

    if complexity > self.max_complexity then
        self:report_diagnostic({
            severity = self.severity,
            message = string.format(
                "関数 '%s' の認知的複雑度が高すぎます（%d、最大%d）",
                func_decl:get_name(), complexity, self.max_complexity
            ),
            location = func_decl:get_location(),
        })
    end
end

function rule:calculate_complexity(stmt, nesting_level)
    nesting_level = nesting_level or 0
    local complexity = 0

    for _, child in ipairs(stmt:get_children()) do
        if child:is_if_stmt() then
            complexity = complexity + 1 + nesting_level
            complexity = complexity + self:calculate_complexity(child, nesting_level + 1)
        elseif child:is_for_stmt() or child:is_while_stmt() then
            complexity = complexity + 1 + nesting_level
            complexity = complexity + self:calculate_complexity(child, nesting_level + 1)
        elseif child:is_switch_stmt() then
            complexity = complexity + 1
            complexity = complexity + self:calculate_complexity(child, nesting_level)
        elseif child:is_binary_operator() then
            local op = child:get_operator()
            if op == "&&" or op == "||" then
                complexity = complexity + 1
            end
            complexity = complexity + self:calculate_complexity(child, nesting_level)
        else
            complexity = complexity + self:calculate_complexity(child, nesting_level)
        end
    end

    return complexity
end

return rule
