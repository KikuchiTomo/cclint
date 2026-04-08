-- ブール変数には説明的な接頭辞を付けるべき (is_, has_, can_, should_ など)
rule_description = "ブール変数には説明的な接頭辞を付けること"
rule_category = "naming"

function check_ast()
    local classes = cclint.get_classes()
    if not classes then return end

    local good_prefixes = {"is_", "has_", "can_", "should_", "will_", "was_", "did_", "needs_", "enable", "disable", "allow"}

    for _, class_name in ipairs(classes) do
        local fields = cclint.get_fields(class_name)
        if fields then
            for _, field_name in ipairs(fields) do
                local field_info = cclint.get_field_info(class_name, field_name)
                if field_info and field_info.type == "bool" then
                    -- Check if it has a good prefix
                    local has_good_prefix = false
                    for _, prefix in ipairs(good_prefixes) do
                        if field_name:match("^" .. prefix) then
                            has_good_prefix = true
                            break
                        end
                    end

                    if not has_good_prefix then
                        cclint.report_info(
                            field_info.line, 1,
                            string.format("ブール変数 '%s' には説明的な接頭辞を付けてください (is_, has_, can_, should_ など)",
                                field_name)
                        )
                    end
                end
            end
        end
    end
end
