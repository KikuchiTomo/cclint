-- 深くネストした条件文を避けるべき
--
-- 説明:
--   深くネストした条件文は可読性を低下させます。
--   早期リターンやガード節を使用して平坦化すべきです。
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度
--   - max_depth: 最大ネスト深度 (デフォルト: 3)

local rule = {
    name = "avoid-nested-conditionals",
    description = "深くネストした条件文を避けるべき",
    severity = "warning",
}

function rule:init(params)
    self.max_depth = params.max_depth or 3
end

function rule:visit_if_stmt(if_stmt)
    local depth = self:calculate_nesting_depth(if_stmt)

    if depth > self.max_depth then
        self:report_diagnostic({
            severity = self.severity,
            message = string.format(
                "条件文のネストが深すぎます（深度: %d、最大: %d）",
                depth, self.max_depth
            ),
            location = if_stmt:get_location(),
            notes = {
                {
                    severity = "info",
                    message = "早期リターンやガード節を使用してネストを減らしてください",
                }
            }
        })
    end
end

function rule:calculate_nesting_depth(stmt)
    local depth = 0
    local current = stmt:get_parent()

    while current do
        if current:is_if_stmt() or current:is_for_stmt() or
           current:is_while_stmt() or current:is_switch_stmt() then
            depth = depth + 1
        end
        current = current:get_parent()
    end

    return depth
end

return rule
