-- rand()の代わりに<random>を使用すべき
--
-- 説明:
--   C言語のrand()関数は暗号学的に安全ではありません。
--   C++11の<random>ライブラリを使用すべきです。
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度

local rule = {
    name = "no-rand",
    description = "rand()の代わりに<random>を使用すべき",
    severity = "warning",
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

    if func_name == "rand" or func_name == "srand" then
        self:report_diagnostic({
            severity = self.severity,
            message = string.format(
                "%s()関数は非推奨です",
                func_name
            ),
            location = call_expr:get_location(),
            notes = {
                {
                    severity = "info",
                    message = "C++11の<random>ライブラリ（std::mt19937など）を使用してください",
                }
            }
        })
    end
end

return rule
