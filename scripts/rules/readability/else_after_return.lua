-- return後のelseは不要
--
-- 説明:
--   if文でreturnした後のelseは冗長です。
--   elseを削除してネストを減らすことができます。
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度

local rule = {
    name = "else-after-return",
    description = "return後のelseは不要",
    severity = "info",
}

function rule:init(params)
    -- パラメータなし
end

function rule:visit_if_stmt(if_stmt)
    -- then節がreturnで終わっているかチェック
    local then_branch = if_stmt:get_then()

    if not self:ends_with_return(then_branch) then
        return
    end

    -- else節があるかチェック
    local else_branch = if_stmt:get_else()

    if not else_branch then
        return
    end

    self:report_diagnostic({
        severity = self.severity,
        message = "return後のelseは不要です",
        location = else_branch:get_location(),
        notes = {
            {
                severity = "info",
                message = "elseを削除してネストを減らすことができます",
            }
        }
    })
end

-- 文がreturnで終わっているかチェック
function rule:ends_with_return(stmt)
    if stmt:is_return_stmt() then
        return true
    end

    if stmt:is_compound_stmt() then
        local children = stmt:get_children()
        if #children > 0 then
            return self:ends_with_return(children[#children])
        end
    end

    return false
end

return rule
