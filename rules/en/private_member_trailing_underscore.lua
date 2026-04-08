-- Private member variables should have trailing underscore (e.g., data_, count_)
rule_description = "Private member variables should have trailing underscore"
rule_category = "naming"

function check_ast()
    local classes = cclint.get_classes()
    if not classes then return end

    for _, class_name in ipairs(classes) do
        local fields = cclint.get_fields(class_name)
        if fields then
            for _, field_name in ipairs(fields) do
                local field_info = cclint.get_field_info(class_name, field_name)
                if field_info then
                    -- Check if private and non-static
                    if field_info.access_value == 2 and not field_info.is_static then
                        -- Should end with underscore
                        if not field_name:match("_$") then
                            cclint.report_warning(
                                field_info.line, 1,
                                string.format("Private member variable '%s' should have trailing underscore (e.g., %s_)",
                                    field_name, field_name)
                            )
                        end
                    end
                end
            end
        end
    end
end
