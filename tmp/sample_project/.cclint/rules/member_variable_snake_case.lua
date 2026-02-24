-- Rule: Class member variables must use lower_snake_case
-- Check that member variable names follow lower_snake_case convention

rule_description = "Member variables must use lower_snake_case naming convention"
rule_category = "naming"

function is_lower_snake_case(name)
    -- Remove trailing underscore if present (for member variable suffix)
    local clean_name = string.gsub(name, "_$", "")

    -- Check if name matches lower_snake_case pattern
    return string.match(clean_name, "^[a-z_][a-z0-9_]*$") ~= nil
end

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

                        -- Skip static constants (usually UPPER_CASE)
                        local is_static_const = field.is_static and field.is_const

                        if field_name and not is_static_const then
                            if not is_lower_snake_case(field_name) then
                                cclint.report_error(
                                    line, 1,
                                    string.format(
                                        "Member variable '%s' must use lower_snake_case naming convention",
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
