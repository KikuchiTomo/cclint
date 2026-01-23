-- #includeの順序を統一
--
-- 説明:
--   #include文は適切な順序で記述すべきです。
--   推奨順序: 1. 対応するヘッダー 2. C標準ライブラリ 3. C++標準ライブラリ 4. その他
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度
--   - style: 順序スタイル ("google", "llvm", "custom")

local rule = {
    name = "include-order",
    description = "#includeの順序を統一",
    severity = "warning",
}

function rule:init(params)
    self.style = params.style or "google"
end

function rule:check_file(file)
    local includes = file:get_include_directives()

    if #includes == 0 then
        return
    end

    -- インクルードをカテゴリ分け
    local categorized = self:categorize_includes(includes)

    -- 順序をチェック
    local expected_order = self:get_expected_order()
    local prev_category = nil

    for _, inc in ipairs(includes) do
        local category = self:get_include_category(inc)

        if prev_category then
            local prev_index = self:find_category_index(expected_order, prev_category)
            local curr_index = self:find_category_index(expected_order, category)

            if curr_index < prev_index then
                self:report_diagnostic({
                    severity = self.severity,
                    message = string.format(
                        "#includeの順序が不正です（%s は %s の後にあるべきです）",
                        category, prev_category
                    ),
                    location = inc:get_location(),
                })
            end
        end

        prev_category = category
    end
end

function rule:get_expected_order()
    if self.style == "google" then
        return {"main", "c_std", "cpp_std", "other", "project"}
    elseif self.style == "llvm" then
        return {"main", "project", "cpp_std", "c_std", "other"}
    else
        return {"main", "c_std", "cpp_std", "project", "other"}
    end
end

function rule:get_include_category(include)
    local path = include:get_included_path()

    -- 対応するヘッダー
    if include:is_main_file_include() then
        return "main"
    end

    -- C標準ライブラリ
    if path:match("^<c[a-z]+>$") or path:match("^<[a-z]+%.h>$") then
        return "c_std"
    end

    -- C++標準ライブラリ
    if path:match("^<[a-z_]+>$") then
        return "cpp_std"
    end

    -- プロジェクトヘッダー (相対パスまたは"")
    if path:match('^"') then
        return "project"
    end

    return "other"
end

function rule:find_category_index(order, category)
    for i, cat in ipairs(order) do
        if cat == category then
            return i
        end
    end
    return #order + 1
end

function rule:categorize_includes(includes)
    local result = {}
    for _, inc in ipairs(includes) do
        local category = self:get_include_category(inc)
        if not result[category] then
            result[category] = {}
        end
        table.insert(result[category], inc)
    end
    return result
end

return rule
