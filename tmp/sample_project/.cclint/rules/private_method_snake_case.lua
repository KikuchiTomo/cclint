-- Rule: Private methods must use lower_snake_case
-- Check that private method names follow lower_snake_case convention

rule_description = "Private methods must use lower_snake_case naming convention"
rule_category = "naming"

function is_lower_snake_case(name)
    -- Remove __ prefix if present (for private methods)
    local clean_name = string.gsub(name, "^__", "")

    -- Check if name matches lower_snake_case pattern
    return string.match(clean_name, "^[a-z_][a-z0-9_]*$") ~= nil
end

function check_ast()
    -- Get all classes
    local classes = cclint.get_classes_with_info()

    if not classes then
        return
    end

    -- For each class, get private methods
    for _, class_info in ipairs(classes) do
        local class_name = class_info.name

        if class_name then
            -- Get all private methods
            local private_methods = cclint.get_class_methods_by_access(class_name, "private")

            if private_methods then
                for _, method in ipairs(private_methods) do
                    local method_name = method.name
                    local line = method.line or 1

                    -- Skip constructors and destructors
                    if method_name and method_name ~= class_name and method_name ~= "~" .. class_name then
                        if not is_lower_snake_case(method_name) then
                            cclint.report_error(
                                line, 1,
                                string.format(
                                    "Private method '%s' must use lower_snake_case naming convention",
                                    method_name
                                )
                            )
                        end
                    end
                end
            end
        end
    end
end
