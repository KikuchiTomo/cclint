-- push_backの代わりにemplace_backを使用すべき
--
-- 説明:
--   コンテナへの要素追加時、push_backよりemplace_backを使用することで
--   不要なコピー/ムーブを避けることができます。
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度

local rule = {
    name = "use-emplace",
    description = "push_backの代わりにemplace_backを使用すべき",
    severity = "info",
}

function rule:init(params)
    -- パラメータなし
end

function rule:visit_call_expr(call_expr)
    local callee = call_expr:get_callee()

    if not callee:is_member_function() then
        return
    end

    local method_name = callee:get_name()

    -- push_back, push_front, insertのチェック
    local emplace_alternative = nil

    if method_name == "push_back" then
        emplace_alternative = "emplace_back"
    elseif method_name == "push_front" then
        emplace_alternative = "emplace_front"
    elseif method_name == "insert" then
        emplace_alternative = "emplace"
    else
        return
    end

    -- 引数が一時オブジェクトかチェック
    local args = call_expr:get_arguments()
    if #args == 0 then
        return
    end

    local arg = args[#args]  -- 最後の引数（値引数）

    -- コンストラクタ呼び出しまたはmake_系関数の場合
    if arg:is_construct_expr() or arg:is_temporary_object() then
        self:report_diagnostic({
            severity = self.severity,
            message = string.format(
                "%sの代わりに%sを使用できます",
                method_name, emplace_alternative
            ),
            location = call_expr:get_location(),
            notes = {
                {
                    severity = "info",
                    message = "不要なコピー/ムーブを避けることができます",
                }
            }
        })
    end
end

return rule
