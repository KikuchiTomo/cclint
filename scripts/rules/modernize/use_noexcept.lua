-- throw()の代わりにnoexceptを使用すべき
--
-- 説明:
--   C++11以降、throw()よりもnoexceptを使用すべきです。
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度

local rule = {
    name = "use-noexcept",
    description = "throw()の代わりにnoexceptを使用すべき",
    severity = "warning",
}

function rule:init(params)
    -- パラメータなし
end

function rule:visit_function_decl(func_decl)
    -- throw()指定子をチェック
    if func_decl:has_exception_spec() and
       func_decl:get_exception_spec_type() == "throw" then

        self:report_diagnostic({
            severity = self.severity,
            message = "throw()の代わりにnoexceptを使用すべきです",
            location = func_decl:get_location(),
            fix_hints = {
                {
                    range = func_decl:get_exception_spec_range(),
                    replacement = "noexcept",
                }
            }
        })
    end
end

return rule
