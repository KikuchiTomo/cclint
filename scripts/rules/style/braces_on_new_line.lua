-- 開き波括弧は新しい行に配置すべき（Allman style）
--
-- 説明:
--   関数定義やクラス定義の開き波括弧は新しい行に配置すべきです。
--
-- 例:
--   // Good
--   void foo()
--   {
--       ...
--   }
--
--   // Bad
--   void foo() {
--       ...
--   }
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度
--   - style: "allman" (新しい行) または "k&r" (同じ行)

local rule = {
    name = "braces-on-new-line",
    description = "開き波括弧は新しい行に配置すべき",
    severity = "warning",
}

function rule:init(params)
    self.style = params.style or "allman"
end

function rule:visit_function_decl(func_decl)
    -- 定義を持つ関数のみチェック
    if not func_decl:has_body() then
        return
    end

    local body = func_decl:get_body()
    local func_loc = func_decl:get_location()
    local brace_loc = body:get_left_brace_location()

    if self.style == "allman" then
        -- 波括弧が同じ行にある場合はエラー
        if func_loc:get_line() == brace_loc:get_line() then
            self:report_diagnostic({
                severity = self.severity,
                message = "開き波括弧は新しい行に配置すべきです",
                location = brace_loc,
            })
        end
    elseif self.style == "k&r" then
        -- 波括弧が別の行にある場合はエラー
        if func_loc:get_line() ~= brace_loc:get_line() then
            self:report_diagnostic({
                severity = self.severity,
                message = "開き波括弧は宣言と同じ行に配置すべきです",
                location = brace_loc,
            })
        end
    end
end

function rule:visit_class_decl(class_decl)
    local body = class_decl:get_body()
    if not body then
        return
    end

    local class_loc = class_decl:get_location()
    local brace_loc = body:get_left_brace_location()

    if self.style == "allman" then
        if class_loc:get_line() == brace_loc:get_line() then
            self:report_diagnostic({
                severity = self.severity,
                message = "開き波括弧は新しい行に配置すべきです",
                location = brace_loc,
            })
        end
    end
end

return rule
