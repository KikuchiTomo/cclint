-- クラス名はCamelCaseであるべき
--
-- 説明:
--   クラス名は大文字で始まるCamelCase形式であるべきです。
--   例: MyClass, HttpServer, JsonParser
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度 ("error", "warning", "info")
--   - allow_underscores: アンダースコアを許可するか (デフォルト: false)

local rule = {
    name = "class-name-camelcase",
    description = "クラス名はCamelCaseであるべき",
    severity = "warning",
}

-- ルールの初期化
function rule:init(params)
    self.allow_underscores = params.allow_underscores or false
end

-- クラス宣言を訪問
function rule:visit_class_decl(class_decl)
    local name = class_decl:get_name()

    -- クラス名が空の場合はスキップ（無名クラス）
    if name == "" then
        return
    end

    -- CamelCaseの検証
    if not self:is_camel_case(name) then
        self:report_diagnostic({
            severity = self.severity,
            message = string.format("クラス名 '%s' はCamelCaseであるべきです", name),
            location = class_decl:get_location(),
            fix_hints = {
                {
                    range = class_decl:get_name_range(),
                    replacement = self:to_camel_case(name),
                }
            }
        })
    end
end

-- CamelCaseか判定
function rule:is_camel_case(name)
    -- 最初の文字が大文字か確認
    if not name:match("^[A-Z]") then
        return false
    end

    -- アンダースコアのチェック
    if not self.allow_underscores and name:match("_") then
        return false
    end

    return true
end

-- CamelCaseに変換（簡易版）
function rule:to_camel_case(name)
    -- アンダースコア区切りをCamelCaseに変換
    local parts = {}
    for part in name:gmatch("[^_]+") do
        table.insert(parts, part:sub(1,1):upper() .. part:sub(2))
    end
    return table.concat(parts, "")
end

return rule
