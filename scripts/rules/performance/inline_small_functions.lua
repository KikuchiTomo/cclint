-- 小さな関数にはinlineを検討すべき
--
-- 説明:
--   非常に小さな関数（1-3行）にはinline指定を検討すべきです。
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度
--   - max_lines: inline推奨する最大行数 (デフォルト: 3)

local rule = {
    name = "inline-small-functions",
    description = "小さな関数にはinlineを検討すべき",
    severity = "info",
}

function rule:init(params)
    self.max_lines = params.max_lines or 3
end

function rule:visit_function_decl(func_decl)
    -- すでにinlineの場合はスキップ
    if func_decl:is_inline() then
        return
    end

    -- テンプレート関数はスキップ（暗黙的inline）
    if func_decl:is_template() then
        return
    end

    -- 定義がない場合はスキップ
    if not func_decl:has_body() then
        return
    end

    local body = func_decl:get_body()
    local start_line = body:get_begin_location():get_line()
    local end_line = body:get_end_location():get_line()
    local lines = end_line - start_line + 1

    if lines <= self.max_lines then
        self:report_diagnostic({
            severity = self.severity,
            message = string.format(
                "関数 '%s' は小さいのでinline化を検討できます（%d行）",
                func_decl:get_name(), lines
            ),
            location = func_decl:get_location(),
        })
    end
end

return rule
