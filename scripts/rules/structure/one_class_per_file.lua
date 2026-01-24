-- 1ファイルに1クラス
--
-- 説明:
--   1つのファイルには1つのpublicクラスのみを定義すべきです。
--   これによりコードの整理と検索が容易になります。
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度
--   - allow_nested: ネストクラスを許可するか (デフォルト: true)
--   - allow_private: 同じファイル内のprivateクラスを許可 (デフォルト: true)

local rule = {
    name = "one-class-per-file",
    description = "1ファイルに1クラス",
    severity = "warning",
}

function rule:init(params)
    self.allow_nested = params.allow_nested ~= false  -- デフォルトtrue
    self.allow_private = params.allow_private ~= false  -- デフォルトtrue
end

function rule:check_file(file)
    local classes = file:get_top_level_classes()
    local public_classes = {}

    for _, class_decl in ipairs(classes) do
        -- ネストクラスのチェック
        if self.allow_nested and class_decl:is_nested() then
            goto continue
        end

        -- privateクラスのチェック
        if self.allow_private and class_decl:get_access() == "private" then
            goto continue
        end

        -- 無名クラスはスキップ
        if class_decl:get_name() == "" then
            goto continue
        end

        table.insert(public_classes, class_decl)

        ::continue::
    end

    -- 複数のpublicクラスがある場合
    if #public_classes > 1 then
        for i, class_decl in ipairs(public_classes) do
            if i > 1 then  -- 最初のクラス以外を警告
                self:report_diagnostic({
                    severity = self.severity,
                    message = string.format(
                        "1ファイルに複数のクラスが定義されています（クラス '%s'）",
                        class_decl:get_name()
                    ),
                    location = class_decl:get_location(),
                    notes = {
                        {
                            severity = "info",
                            message = string.format(
                                "最初のクラス '%s' がファイル '%s' で定義されています",
                                public_classes[1]:get_name(),
                                file:get_filename()
                            ),
                            location = public_classes[1]:get_location(),
                        }
                    }
                })
            end
        end
    end

    -- ファイル名とクラス名の一致もチェック
    if #public_classes == 1 then
        local class_name = public_classes[1]:get_name()
        local filename = file:get_stem()  -- 拡張子を除いたファイル名

        if class_name ~= filename then
            self:report_diagnostic({
                severity = self.severity,
                message = string.format(
                    "ファイル名 '%s' とクラス名 '%s' が一致しません",
                    filename, class_name
                ),
                location = public_classes[1]:get_location(),
                notes = {
                    {
                        severity = "info",
                        message = string.format(
                            "ファイル名を '%s.cpp' または '%s.hpp' に変更してください",
                            class_name, class_name
                        ),
                    }
                }
            })
        end
    end
end

return rule
