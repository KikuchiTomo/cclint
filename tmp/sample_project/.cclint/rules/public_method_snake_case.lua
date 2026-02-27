-- Rule: Public methods must use lower_snake_case
-- Check that public method names follow lower_snake_case convention

rule_description = "Public methods must use lower_snake_case naming convention"
rule_category = "naming"

function is_lower_snake_case(name)
    -- Check if name matches lower_snake_case pattern
    return string.match(name, "^[a-z_][a-z0-9_]*$") ~= nil
end

function check_ast()
    -- Get all classes
    local classes = cclint.get_classes_with_info()

    if not classes then
        return
    end

    -- For each class, get public methods
    for _, class_info in ipairs(classes) do
        local class_name = class_info.name

        if class_name then
            -- Get all public methods
            local public_methods = cclint.get_class_methods_by_access(class_name, "public")

            if public_methods then
                for _, method in ipairs(public_methods) do
                    local method_name = method.name
                    local line = method.line or 1

                    -- Skip constructors and destructors
                    if method_name and method_name ~= class_name and method_name ~= "~" .. class_name then
                        if not is_lower_snake_case(method_name) then
                            cclint.report_error(
                                line, 1,
                                string.format(
                                    "Public method '%s' must use lower_snake_case naming convention",
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
