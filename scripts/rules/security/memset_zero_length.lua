-- memsetの長さゼロは無意味
--
-- 説明:
--   memsetで長さ0を指定するのは誤りの可能性が高いです。
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度

local rule = {
    name = "memset-zero-length",
    description = "memsetの長さゼロは無意味",
    severity = "error",
}

function rule:init(params)
    -- パラメータなし
end

function rule:visit_call_expr(call_expr)
    local callee = call_expr:get_callee()

    if not callee:is_function_decl() then
        return
    end

    local func_name = callee:get_name()

    if func_name == "memset" or func_name == "memcpy" or func_name == "memmove" then
        local args = call_expr:get_arguments()

        -- 引数チェック
        if #args < 3 then
            return
        end

        -- 3番目の引数（サイズ）をチェック
        local size_arg = args[3]

        if size_arg:is_integer_literal() and size_arg:get_value() == 0 then
            self:report_diagnostic({
                severity = self.severity,
                message = string.format(
                    "%s()のサイズが0です",
                    func_name
                ),
                location = call_expr:get_location(),
                notes = {
                    {
                        severity = "info",
                        message = "サイズが0の場合、この呼び出しは無意味です",
                    }
                }
            })
        end
    end
end

return rule
