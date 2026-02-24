-- Rule: Private methods must start with '__'
-- Check that all private methods in a class start with double underscore

rule_description = "Private methods must start with '__' (double underscore)"
rule_category = "naming"

function check_ast()
    -- Get all classes with detailed info
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
                        -- Check if method name starts with '__'
                        if not string.match(method_name, "^__") then
                            cclint.report_error(
                                line, 1,
                                string.format(
                                    "Private method '%s' must start with '__' (double underscore)",
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
