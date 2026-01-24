-- vectorにreserve()を使用すべき
--
-- 説明:
--   ループ内でvectorに要素を追加する場合、
--   事前にreserve()でメモリを確保すべきです。
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度
--   - min_push_count: チェックする最小push回数 (デフォルト: 5)

local rule = {
    name = "reserve-vector",
    description = "vectorにreserve()を使用すべき",
    severity = "info",
}

function rule:init(params)
    self.min_push_count = params.min_push_count or 5
end

function rule:visit_for_stmt(for_stmt)
    local body = for_stmt:get_body()

    -- ループ内のvector push_backを検出
    local vector_pushes = self:find_vector_pushes(body)

    for vec_var, count in pairs(vector_pushes) do
        if count >= self.min_push_count then
            -- reserve呼び出しがあるかチェック
            if not self:has_reserve_call(for_stmt, vec_var) then
                self:report_diagnostic({
                    severity = self.severity,
                    message = string.format(
                        "vector '%s' にはループ前にreserve()を呼び出すべきです",
                        vec_var:get_name()
                    ),
                    location = for_stmt:get_location(),
                })
            end
        end
    end
end

function rule:find_vector_pushes(stmt)
    local result = {}

    for _, child in ipairs(stmt:get_children()) do
        if child:is_call_expr() and child:is_member_call() then
            local method = child:get_callee()
            if method:get_name() == "push_back" then
                local obj = child:get_implicit_object_argument()
                if obj:is_decl_ref() then
                    local var = obj:get_referenced_decl()
                    local var_type = var:get_type()

                    -- vectorかチェック
                    if var_type:is_template_instantiation() and
                       var_type:get_template_name() == "vector" then
                        result[var] = (result[var] or 0) + 1
                    end
                end
            end
        end

        -- 再帰的に検索
        local nested = self:find_vector_pushes(child)
        for var, count in pairs(nested) do
            result[var] = (result[var] or 0) + count
        end
    end

    return result
end

function rule:has_reserve_call(for_stmt, vec_var)
    -- ループの前のステートメントをチェック
    local parent = for_stmt:get_parent()
    if not parent or not parent:is_compound_stmt() then
        return false
    end

    local stmts = parent:get_statements()
    local for_index = nil

    -- for文のインデックスを見つける
    for i, stmt in ipairs(stmts) do
        if stmt == for_stmt then
            for_index = i
            break
        end
    end

    if not for_index or for_index == 1 then
        return false
    end

    -- 直前のステートメントでreserve呼び出しをチェック
    local prev_stmt = stmts[for_index - 1]
    return self:contains_reserve_call(prev_stmt, vec_var)
end

function rule:contains_reserve_call(stmt, vec_var)
    if stmt:is_call_expr() and stmt:is_member_call() then
        local method = stmt:get_callee()
        if method:get_name() == "reserve" then
            local obj = stmt:get_implicit_object_argument()
            if obj:is_decl_ref() and obj:get_referenced_decl() == vec_var then
                return true
            end
        end
    end

    for _, child in ipairs(stmt:get_children()) do
        if self:contains_reserve_call(child, vec_var) then
            return true
        end
    end

    return false
end

return rule
