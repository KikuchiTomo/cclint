-- 変更されないパラメータはconst参照にすべき
--
-- 説明:
--   関数内で変更されないパラメータは、
--   値渡しではなくconst参照にすべきです。
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度
--   - min_size: チェック対象の最小サイズ (デフォルト: 16)

local rule = {
    name = "unnecessary-value-param",
    description = "変更されないパラメータはconst参照にすべき",
    severity = "warning",
}

function rule:init(params)
    self.min_size = params.min_size or 16
end

function rule:visit_function_decl(func_decl)
    if not func_decl:has_body() then
        return
    end

    for _, param in ipairs(func_decl:get_parameters()) do
        local param_type = param:get_type()

        -- すでに参照/ポインタの場合はスキップ
        if param_type:is_reference() or param_type:is_pointer() then
            goto continue
        end

        -- プリミティブ型はスキップ
        if param_type:is_builtin() then
            goto continue
        end

        -- 小さい型はスキップ
        if param_type:get_size_in_bytes() < self.min_size then
            goto continue
        end

        -- パラメータが変更されているかチェック
        local body = func_decl:get_body()
        if self:is_parameter_modified(body, param) then
            goto continue
        end

        -- const参照を提案
        self:report_diagnostic({
            severity = self.severity,
            message = string.format(
                "パラメータ '%s' はconst参照にすべきです",
                param:get_name()
            ),
            location = param:get_location(),
            fix_hints = {
                {
                    range = param:get_type_range(),
                    replacement = string.format("const %s&", param_type:get_name()),
                }
            }
        })

        ::continue::
    end
end

function rule:is_parameter_modified(stmt, param)
    for _, child in ipairs(stmt:get_children()) do
        -- 代入のチェック
        if child:is_assignment() then
            local lhs = child:get_lhs()
            if lhs:is_decl_ref() and lhs:get_referenced_decl() == param then
                return true
            end
        end

        -- 非constメンバ関数呼び出しのチェック
        if child:is_call_expr() and child:is_member_call() then
            local obj = child:get_implicit_object_argument()
            if obj:is_decl_ref() and obj:get_referenced_decl() == param then
                local method = child:get_callee()
                if not method:is_const() then
                    return true
                end
            end
        end

        -- 再帰的にチェック
        if self:is_parameter_modified(child, param) then
            return true
        end
    end

    return false
end

return rule
