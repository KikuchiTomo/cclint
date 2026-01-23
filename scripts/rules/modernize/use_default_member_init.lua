-- メンバ変数はデフォルトメンバ初期化を使用すべき
--
-- 説明:
--   C++11以降、メンバ変数の初期化はクラス定義内で行うことができます。
--   コンストラクタ初期化リストよりも明確です。
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度

local rule = {
    name = "use-default-member-init",
    description = "メンバ変数はデフォルトメンバ初期化を使用すべき",
    severity = "info",
}

function rule:init(params)
    -- パラメータなし
end

function rule:visit_class_decl(class_decl)
    local fields = class_decl:get_fields()

    for _, field in ipairs(fields) do
        -- すでに初期化されている場合はスキップ
        if field:has_in_class_initializer() then
            goto continue
        end

        -- 全てのコンストラクタで同じ値に初期化されているかチェック
        local init_value = self:get_common_init_value(class_decl, field)

        if init_value then
            self:report_diagnostic({
                severity = self.severity,
                message = string.format(
                    "メンバ変数 '%s' はデフォルトメンバ初期化を使用できます",
                    field:get_name()
                ),
                location = field:get_location(),
                notes = {
                    {
                        severity = "info",
                        message = string.format(
                            "クラス定義内で初期化してください: %s = %s;",
                            field:get_name(), init_value
                        ),
                    }
                }
            })
        end

        ::continue::
    end
end

-- 全てのコンストラクタで共通の初期化値を取得
function rule:get_common_init_value(class_decl, field)
    local constructors = class_decl:get_constructors()

    if #constructors == 0 then
        return nil
    end

    local common_value = nil

    for _, ctor in ipairs(constructors) do
        local init_value = self:get_field_init_value(ctor, field)

        if not init_value then
            return nil  -- 初期化されていないコンストラクタがある
        end

        if common_value == nil then
            common_value = init_value
        elseif common_value ~= init_value then
            return nil  -- 異なる値で初期化されている
        end
    end

    return common_value
end

-- コンストラクタ内でのフィールド初期化値を取得
function rule:get_field_init_value(ctor, field)
    local init_list = ctor:get_initializer_list()

    for _, init in ipairs(init_list) do
        if init:get_member() == field then
            -- 初期化式を文字列として取得
            return init:get_init_expr():get_source_text()
        end
    end

    return nil
end

return rule
