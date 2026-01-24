-- 大きなオブジェクトはconst参照で渡すべき
--
-- 説明:
--   大きなオブジェクト（クラス、構造体、コンテナ）を関数に渡す際は、
--   値渡しではなくconst参照で渡すべきです。これによりコピーコストを削減できます。
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度
--   - min_size_bytes: チェックを開始する最小サイズ（バイト、デフォルト: 16）
--   - ignore_trivial: トリビアルコピー可能な型を無視 (デフォルト: false)

local rule = {
    name = "pass-by-const-reference",
    description = "大きなオブジェクトはconst参照で渡すべき",
    severity = "warning",
}

function rule:init(params)
    self.min_size_bytes = params.min_size_bytes or 16
    self.ignore_trivial = params.ignore_trivial or false
end

function rule:visit_function_decl(func_decl)
    for i, param in ipairs(func_decl:get_parameters()) do
        local param_type = param:get_type()

        -- 参照型またはポインタ型の場合はスキップ
        if param_type:is_reference() or param_type:is_pointer() then
            goto continue
        end

        -- プリミティブ型の場合はスキップ
        if param_type:is_builtin() then
            goto continue
        end

        -- トリビアルコピー可能な型を無視する設定の場合
        if self.ignore_trivial and param_type:is_trivially_copyable() then
            goto continue
        end

        -- 型のサイズをチェック
        local size = param_type:get_size_in_bytes()
        if size >= self.min_size_bytes then
            -- 関数内で変更されているかチェック
            local is_modified = self:is_parameter_modified(func_decl, param)

            if is_modified then
                -- 変更される場合は非const参照を提案
                self:report_diagnostic({
                    severity = self.severity,
                    message = string.format(
                        "パラメータ '%s' は参照で渡すべきです（サイズ: %d バイト）",
                        param:get_name(), size
                    ),
                    location = param:get_location(),
                    notes = {
                        {
                            severity = "info",
                            message = "関数内で変更されているため、非const参照を使用してください",
                        }
                    }
                })
            else
                -- 変更されない場合はconst参照を提案
                self:report_diagnostic({
                    severity = self.severity,
                    message = string.format(
                        "パラメータ '%s' はconst参照で渡すべきです（サイズ: %d バイト）",
                        param:get_name(), size
                    ),
                    location = param:get_location(),
                    fix_hints = {
                        {
                            range = param:get_type_range(),
                            replacement = string.format("const %s&", param_type:get_name()),
                        }
                    }
                })
            end
        end

        ::continue::
    end
end

-- パラメータが関数内で変更されているかチェック
function rule:is_parameter_modified(func_decl, param)
    if not func_decl:has_body() then
        return false
    end

    local body = func_decl:get_body()
    local param_name = param:get_name()

    -- 関数本体内でパラメータへの代入があるかチェック
    for _, stmt in ipairs(body:get_statements()) do
        if stmt:is_assignment() then
            local lhs = stmt:get_lhs()
            if lhs:is_decl_ref() and lhs:get_referenced_decl() == param then
                return true
            end
        end
    end

    return false
end

return rule
