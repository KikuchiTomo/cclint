-- 型が明らかな場合はautoを使用すべき
--
-- 説明:
--   初期化式から型が明らかな場合はautoを使用することで、
--   コードの保守性と可読性が向上します。
--
-- 例:
--   // Good
--   auto ptr = std::make_unique<Foo>();
--   auto value = static_cast<int>(x);
--
--   // Bad
--   std::unique_ptr<Foo> ptr = std::make_unique<Foo>();
--   int value = static_cast<int>(x);
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度
--   - min_cpp_standard: 最小C++標準 (デフォルト: "c++11")

local rule = {
    name = "use-auto",
    description = "型が明らかな場合はautoを使用すべき",
    severity = "info",
}

function rule:init(params)
    self.min_cpp_standard = params.min_cpp_standard or "c++11"
end

function rule:visit_var_decl(var_decl)
    -- 初期化されていない変数はスキップ
    if not var_decl:has_init() then
        return
    end

    -- すでにautoを使っている場合はスキップ
    if var_decl:get_type():is_auto() then
        return
    end

    local init_expr = var_decl:get_init()

    -- new式
    if init_expr:is_new_expr() then
        self:suggest_auto(var_decl, "new式から型が明らかです")
        return
    end

    -- make_unique, make_shared
    if init_expr:is_call_expr() then
        local callee = init_expr:get_callee()
        if callee:is_function_decl() then
            local func_name = callee:get_name()
            if func_name:match("^make_unique") or
               func_name:match("^make_shared") or
               func_name:match("^make_pair") then
                self:suggest_auto(var_decl, func_name .. "から型が明らかです")
                return
            end
        end
    end

    -- キャスト式
    if init_expr:is_cast_expr() then
        local cast_kind = init_expr:get_cast_kind()
        if cast_kind == "static_cast" or
           cast_kind == "dynamic_cast" or
           cast_kind == "reinterpret_cast" then
            self:suggest_auto(var_decl, "キャスト式から型が明らかです")
            return
        end
    end

    -- コンストラクタ呼び出し
    if init_expr:is_construct_expr() then
        local var_type = var_decl:get_type()
        local init_type = init_expr:get_type()

        -- 型が完全に一致する場合
        if var_type:get_canonical_type() == init_type:get_canonical_type() then
            self:suggest_auto(var_decl, "コンストラクタから型が明らかです")
            return
        end
    end
end

function rule:suggest_auto(var_decl, reason)
    self:report_diagnostic({
        severity = self.severity,
        message = string.format(
            "変数 '%s' の宣言にautoを使用できます",
            var_decl:get_name()
        ),
        location = var_decl:get_location(),
        notes = {
            {
                severity = "info",
                message = reason,
            }
        },
        fix_hints = {
            {
                range = var_decl:get_type_range(),
                replacement = "auto",
            }
        }
    })
end

return rule
