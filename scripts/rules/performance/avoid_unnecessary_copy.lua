-- 不要なコピーを避けるべき
--
-- 説明:
--   ループ内での不要なコピーはパフォーマンスの低下を招きます。
--   範囲for文では可能な限りconst参照を使用すべきです。
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度

local rule = {
    name = "avoid-unnecessary-copy",
    description = "不要なコピーを避けるべき",
    severity = "warning",
}

function rule:init(params)
    -- パラメータなし
end

function rule:visit_range_for_stmt(range_for)
    local loop_var = range_for:get_loop_variable()
    local loop_var_type = loop_var:get_type()

    -- すでに参照型の場合はスキップ
    if loop_var_type:is_reference() or loop_var_type:is_pointer() then
        return
    end

    -- プリミティブ型の場合はスキップ
    if loop_var_type:is_builtin() then
        return
    end

    -- autoの場合は別途チェック
    if loop_var_type:is_auto() then
        -- auto&やconst auto&を推奨
        self:report_diagnostic({
            severity = "info",
            message = string.format(
                "範囲for文の変数 '%s' には参照を使用すべきです",
                loop_var:get_name()
            ),
            location = loop_var:get_location(),
            notes = {
                {
                    severity = "info",
                    message = "const auto& を使用してコピーを避けてください",
                }
            }
        })
        return
    end

    -- ループ内で変数が変更されているかチェック
    local body = range_for:get_body()
    local is_modified = self:is_variable_modified(body, loop_var)

    if is_modified then
        -- 変更される場合は非const参照を提案
        self:report_diagnostic({
            severity = self.severity,
            message = string.format(
                "範囲for文の変数 '%s' は参照で受け取るべきです",
                loop_var:get_name()
            ),
            location = loop_var:get_location(),
            fix_hints = {
                {
                    range = loop_var:get_type_range(),
                    replacement = loop_var_type:get_name() .. "&",
                }
            }
        })
    else
        -- 変更されない場合はconst参照を提案
        self:report_diagnostic({
            severity = self.severity,
            message = string.format(
                "範囲for文の変数 '%s' はconst参照で受け取るべきです",
                loop_var:get_name()
            ),
            location = loop_var:get_location(),
            fix_hints = {
                {
                    range = loop_var:get_type_range(),
                    replacement = "const " .. loop_var_type:get_name() .. "&",
                }
            }
        })
    end
end

function rule:is_variable_modified(stmt, var_decl)
    -- 文内で変数が変更されているかチェック
    -- これは簡略化された実装
    for _, child in ipairs(stmt:get_children()) do
        if child:is_assignment() then
            local lhs = child:get_lhs()
            if lhs:is_decl_ref() and lhs:get_referenced_decl() == var_decl then
                return true
            end
        end

        -- 再帰的にチェック
        if self:is_variable_modified(child, var_decl) then
            return true
        end
    end

    return false
end

return rule
