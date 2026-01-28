-- Static const member variables should use UPPER_CASE (e.g., MAX_SIZE, DEFAULT_VALUE)
rule_description = "Static const members should use UPPER_CASE"
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
                    -- Check if static and const
                    if field_info.is_static and field_info.is_const then
                        -- Should be UPPER_CASE with underscores
                        if not field_name:match("^[A-Z][A-Z0-9_]*$") then
                            cclint.report_warning(
                                field_info.line, 1,
                                string.format("Static const member '%s' should use UPPER_CASE (e.g., %s)",
                                    field_name, field_name:upper())
                            )
                        end
                    end
                end
            end
        end
    end
end
