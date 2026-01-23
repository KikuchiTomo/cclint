-- 定数名はUPPER_CASEであるべき
--
-- 説明:
--   定数（const変数、constexpr変数、マクロ定数）は大文字とアンダースコアを
--   使ったUPPER_CASE形式であるべきです。
--   例: MAX_SIZE, DEFAULT_TIMEOUT, PI_VALUE
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度
--   - apply_to_enum: 列挙値にも適用するか (デフォルト: true)

local rule = {
    name = "constant-name-uppercase",
    description = "定数名はUPPER_CASEであるべき",
    severity = "warning",
}

function rule:init(params)
    self.apply_to_enum = params.apply_to_enum ~= false
end

function rule:visit_var_decl(var_decl)
    -- const/constexpr変数のみチェック
    if not var_decl:is_const() and not var_decl:is_constexpr() then
        return
    end

    -- ローカル変数は除外（関数パラメータやローカル変数）
    if var_decl:is_local() then
        return
    end

    local name = var_decl:get_name()

    -- UPPER_CASEの検証
    if not self:is_upper_case(name) then
        self:report_diagnostic({
            severity = self.severity,
            message = string.format("定数名 '%s' はUPPER_CASEであるべきです", name),
            location = var_decl:get_location(),
            fix_hints = {
                {
                    range = var_decl:get_name_range(),
                    replacement = self:to_upper_case(name),
                }
            }
        })
    end
end

function rule:visit_enum_constant(enum_const)
    if not self.apply_to_enum then
        return
    end

    local name = enum_const:get_name()

    if not self:is_upper_case(name) then
        self:report_diagnostic({
            severity = self.severity,
            message = string.format("列挙値 '%s' はUPPER_CASEであるべきです", name),
            location = enum_const:get_location(),
        })
    end
end

function rule:is_upper_case(name)
    return name:match("^[A-Z][A-Z0-9_]*$") ~= nil
end

function rule:to_upper_case(name)
    -- camelCaseやsnake_caseをUPPER_CASEに変換
    local result = name:gsub("([a-z])([A-Z])", "%1_%2")  -- camelCase対応
    return result:upper()
end

return rule
