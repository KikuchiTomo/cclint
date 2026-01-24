-- switch文にはdefaultケースが必要
--
-- 説明:
--   switch文には常にdefaultケースを含めるべきです。
--   予期しない値に対する適切な処理を保証します。
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度
--   - allow_covered_enum: 全ケースをカバーする列挙型は除外 (デフォルト: true)

local rule = {
    name = "switch-has-default",
    description = "switch文にはdefaultケースが必要",
    severity = "warning",
}

function rule:init(params)
    self.allow_covered_enum = params.allow_covered_enum ~= false
end

function rule:visit_switch_stmt(switch_stmt)
    -- defaultケースがあるかチェック
    if switch_stmt:has_default_case() then
        return
    end

    -- 列挙型の完全カバレッジチェック
    if self.allow_covered_enum then
        local cond = switch_stmt:get_condition()
        local cond_type = cond:get_type()

        if cond_type:is_enum_type() then
            local enum_decl = cond_type:get_declaration()
            local enum_constants = enum_decl:get_enum_constants()
            local case_count = switch_stmt:get_case_count()

            -- 全ての列挙値がカバーされている場合
            if #enum_constants == case_count then
                return
            end
        end
    end

    self:report_diagnostic({
        severity = self.severity,
        message = "switch文にdefaultケースがありません",
        location = switch_stmt:get_location(),
        notes = {
            {
                severity = "info",
                message = "予期しない値に対する処理を追加してください",
            }
        }
    })
end

return rule
