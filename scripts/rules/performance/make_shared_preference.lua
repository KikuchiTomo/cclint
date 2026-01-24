-- new shared_ptrよりmake_sharedを使用すべき
--
-- 説明:
--   make_sharedを使用することでメモリ割り当てが最適化されます。
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度

local rule = {
    name = "make-shared-preference",
    description = "new shared_ptrよりmake_sharedを使用すべき",
    severity = "info",
}

function rule:init(params)
    -- パラメータなし
end

function rule:visit_construct_expr(construct_expr)
    local type_name = construct_expr:get_type():get_name()

    -- shared_ptrの構築かチェック
    if not type_name:match("shared_ptr") then
        return
    end

    -- 引数をチェック
    local args = construct_expr:get_arguments()
    if #args == 0 then
        return
    end

    -- new式で初期化されているかチェック
    if args[1]:is_new_expr() then
        self:report_diagnostic({
            severity = self.severity,
            message = "new式でshared_ptrを初期化しています",
            location = construct_expr:get_location(),
            notes = {
                {
                    severity = "info",
                    message = "std::make_sharedを使用してください",
                }
            }
        })
    end
end

return rule
