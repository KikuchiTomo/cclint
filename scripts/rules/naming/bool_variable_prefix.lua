-- Boolean variables should have descriptive names (is_, has_, can_, should_, etc.)
rule_description = "Boolean variables should have descriptive prefixes"
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
                            string.format("Boolean variable '%s' should have descriptive prefix (is_, has_, can_, should_, etc.)",
                                field_name)
                        )
                    end
                end
            end
        end
    end
end
