-- Class names must use PascalCase (e.g., MyClass, HttpServer)
rule_description = "Class names must use PascalCase"
rule_category = "naming"

function check_ast()
    local classes = cclint.get_classes()
    if not classes then return end

    for _, class_name in ipairs(classes) do
        -- PascalCase: starts with uppercase, contains only alphanumeric
        if not class_name:match("^[A-Z][a-zA-Z0-9]*$") then
            local info = cclint.get_class_info(class_name)
            if info then
                cclint.report_warning(
                    info.line, 1,
                    string.format("Class name '%s' should use PascalCase (e.g., MyClass)", class_name)
                )
            end
        end
    end
end
