-- Rule: Class member variables must end with '_'
-- Check that member variable names end with underscore

rule_description = "Member variables must end with '_' (underscore)"
rule_category = "naming"

function check_ast()
    -- Get all classes
    local classes = cclint.get_classes_with_info()

    if not classes then
        return
    end

    -- For each class, get member variables
    for _, class_info in ipairs(classes) do
        local class_name = class_info.name

        if class_name then
            -- Get all member fields (private, public, protected)
            for _, access in ipairs({"public", "private", "protected"}) do
                local fields = cclint.get_class_fields_by_access(class_name, access)

                if fields then
                    for _, field in ipairs(fields) do
                        local field_name = field.name
                        local line = field.line or 1

                        -- Skip static constants (usually don't need trailing underscore)
                        local is_static_const = field.is_static and field.is_const

                        if field_name and not is_static_const then
                            -- Check if variable name ends with '_'
                            if not string.match(field_name, "_$") then
                                cclint.report_error(
                                    line, 1,
                                    string.format(
                                        "Member variable '%s' must end with '_' (underscore)",
                                        field_name
                                    )
                                )
                            end
                        end
                    end
                end
            end
        end
    end
end
