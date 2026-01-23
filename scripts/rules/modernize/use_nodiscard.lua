-- 戻り値が重要な関数には[[nodiscard]]を付けるべき
--
-- 説明:
--   戻り値を無視すべきでない関数には[[nodiscard]]属性を付けるべきです。
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度
--   - apply_to_all_non_void: 全ての非void関数に適用 (デフォルト: false)

local rule = {
    name = "use-nodiscard",
    description = "戻り値が重要な関数には[[nodiscard]]を付けるべき",
    severity = "info",
}

function rule:init(params)
    self.apply_to_all_non_void = params.apply_to_all_non_void or false
end

function rule:visit_function_decl(func_decl)
    -- void関数はスキップ
    local return_type = func_decl:get_return_type()
    if return_type:is_void() then
        return
    end

    -- すでにnodiscard属性がある場合はスキップ
    if func_decl:has_attribute("nodiscard") then
        return
    end

    -- コンストラクタ/デストラクタはスキップ
    if func_decl:is_constructor() or func_decl:is_destructor() then
        return
    end

    -- setter系の関数はスキップ
    local name = func_decl:get_name()
    if name:match("^set") or name:match("^Set") then
        return
    end

    -- 推奨される場合
    local should_have_nodiscard = false

    if self.apply_to_all_non_void then
        should_have_nodiscard = true
    else
        -- 特定のパターンのみ
        -- getter系関数
        if name:match("^get") or name:match("^Get") or name:match("^is") or name:match("^Is") then
            should_have_nodiscard = true
        end

        -- constメンバ関数でポインタ以外を返す
        if func_decl:is_const() and not return_type:is_pointer() and not return_type:is_void() then
            should_have_nodiscard = true
        end
    end

    if should_have_nodiscard then
        self:report_diagnostic({
            severity = self.severity,
            message = string.format(
                "関数 '%s' には[[nodiscard]]属性を付けるべきです",
                name
            ),
            location = func_decl:get_location(),
            notes = {
                {
                    severity = "info",
                    message = "戻り値の無視を防ぐために[[nodiscard]]を追加してください",
                }
            }
        })
    end
end

return rule
