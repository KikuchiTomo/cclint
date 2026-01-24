-- simple_class_check.lua
-- クラス名がPascalCaseであることをチェックする簡単な例

rule_description = "Check that class names use PascalCase"
rule_category = "naming"

function check_ast(file_path)
    -- すべてのクラスを取得
    local classes = cclint.get_classes()

    if not classes then
        return
    end

    -- 各クラス名をチェック
    for i = 1, #classes do
        local class_name = classes[i]
        local info = cclint.get_class_info(class_name)

        if info then
            -- PascalCaseパターン（大文字で始まる）
            if not string.match(class_name, "^[A-Z][a-zA-Z0-9]*$") then
                cclint.report_warning(
                    info.line or 0, 0,
                    string.format(
                        "Class '%s' should use PascalCase (e.g., MyClass, DataProcessor)",
                        class_name
                    )
                )
            end

            -- structの場合は追加の警告
            if info.is_struct then
                cclint.report_info(
                    info.line or 0, 0,
                    string.format("Found struct '%s'", class_name)
                )
            end
        end
    end
end
