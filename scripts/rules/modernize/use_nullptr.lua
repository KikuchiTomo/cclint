-- NULLや0の代わりにnullptrを使用すべき
--
-- 説明:
--   C++11以降では、ポインタのnull値にはnullptrを使用すべきです。
--   NULLや0は型安全性が低いため推奨されません。
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度
--   - min_cpp_standard: 最小C++標準（デフォルト: "c++11"）

local rule = {
    name = "use-nullptr",
    description = "NULLや0の代わりにnullptrを使用すべき",
    severity = "warning",
}

function rule:init(params)
    self.min_cpp_standard = params.min_cpp_standard or "c++11"
end

function rule:visit_integer_literal(literal)
    -- ポインタコンテキストでの0をチェック
    if literal:get_value() == 0 and literal:is_in_pointer_context() then
        self:report_diagnostic({
            severity = self.severity,
            message = "0の代わりにnullptrを使用してください",
            location = literal:get_location(),
            fix_hints = {
                {
                    range = literal:get_range(),
                    replacement = "nullptr",
                }
            }
        })
    end
end

function rule:visit_macro_expansion(macro)
    -- NULLマクロの使用をチェック
    if macro:get_name() == "NULL" and macro:is_in_pointer_context() then
        self:report_diagnostic({
            severity = self.severity,
            message = "NULLの代わりにnullptrを使用してください",
            location = macro:get_location(),
            fix_hints = {
                {
                    range = macro:get_range(),
                    replacement = "nullptr",
                }
            }
        })
    end
end

return rule
