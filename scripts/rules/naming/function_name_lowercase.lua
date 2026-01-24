-- 関数名はlower_case（スネークケース）であるべき
--
-- 説明:
--   関数名は小文字とアンダースコアを使ったスネークケース形式であるべきです。
--   例: calculate_sum, get_user_name, process_data
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度 ("error", "warning", "info")
--   - allow_camelcase: camelCaseも許可するか (デフォルト: false)

local rule = {
    name = "function-name-lowercase",
    description = "関数名はlower_caseであるべき",
    severity = "warning",
}

function rule:init(params)
    self.allow_camelcase = params.allow_camelcase or false
end

function rule:visit_function_decl(func_decl)
    local name = func_decl:get_name()

    -- 空の名前はスキップ
    if name == "" then
        return
    end

    -- 特殊な関数名をスキップ（演算子オーバーロードなど）
    if name:match("^operator") then
        return
    end

    -- コンストラクタ/デストラクタをスキップ
    if func_decl:is_constructor() or func_decl:is_destructor() then
        return
    end

    -- スネークケースの検証
    if not self:is_snake_case(name) then
        if self.allow_camelcase and self:is_camel_case(name) then
            return
        end

        self:report_diagnostic({
            severity = self.severity,
            message = string.format("関数名 '%s' はlower_caseであるべきです", name),
            location = func_decl:get_location(),
        })
    end
end

function rule:is_snake_case(name)
    -- 小文字、数字、アンダースコアのみ
    return name:match("^[a-z][a-z0-9_]*$") ~= nil
end

function rule:is_camel_case(name)
    -- 小文字で始まり、大文字も含む
    return name:match("^[a-z][a-zA-Z0-9]*$") ~= nil
end

return rule
