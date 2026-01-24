-- 安全でない関数の使用を禁止
--
-- 説明:
--   strcpy, sprintf, getsなどの安全でない関数の使用を検出します。
--   これらの関数はバッファオーバーフローの原因となります。
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度
--   - unsafe_functions: 禁止する関数のリスト（カスタマイズ可能）

local rule = {
    name = "no-unsafe-functions",
    description = "安全でない関数の使用を禁止",
    severity = "error",
}

-- デフォルトの安全でない関数リスト
local default_unsafe_functions = {
    strcpy = "strncpy または std::string を使用してください",
    strcat = "strncat または std::string を使用してください",
    sprintf = "snprintf または std::stringstream を使用してください",
    vsprintf = "vsnprintf を使用してください",
    gets = "fgets または std::getline を使用してください",
    scanf = "より安全な入力方法を使用してください",
    sscanf = "より安全なパース方法を使用してください",
    atoi = "std::stoi（例外処理付き）を使用してください",
    atol = "std::stol（例外処理付き）を使用してください",
    atof = "std::stod（例外処理付き）を使用してください",
}

function rule:init(params)
    self.unsafe_functions = params.unsafe_functions or default_unsafe_functions
end

function rule:visit_call_expr(call_expr)
    local callee = call_expr:get_callee()

    if not callee:is_function_decl() then
        return
    end

    local func_name = callee:get_name()

    -- 安全でない関数のチェック
    local suggestion = self.unsafe_functions[func_name]
    if suggestion then
        self:report_diagnostic({
            severity = self.severity,
            message = string.format(
                "安全でない関数 '%s' の使用は禁止されています",
                func_name
            ),
            location = call_expr:get_location(),
            notes = {
                {
                    severity = "info",
                    message = suggestion,
                }
            }
        })
    end
end

return rule
