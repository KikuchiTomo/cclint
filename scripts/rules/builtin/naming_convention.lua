-- Naming Convention Rule (Lua version, replaces C++ builtin)
rule_description = "Check naming conventions for classes, functions, and variables"
rule_category = "naming"

function check_ast()
    -- Check class names (should be PascalCase)
    local classes = cclint.get_classes()
    if classes then
        for i, class_name in ipairs(classes) do
            if not class_name:match("^[A-Z][a-zA-Z0-9]*$") then
                local info = cclint.get_class_info(class_name)
                if info then
                    cclint.report_warning(
                        info.line, 1,
                        string.format("Class name '%s' should use PascalCase", class_name)
                    )
                end
            end
        end
    end

    -- Check function names (should be snake_case)
    local functions = cclint.get_functions()
    if functions then
        for i, func in ipairs(functions) do
            -- Skip constructors and destructors
            if not (func.name:match("^~") or func.name:match("^[A-Z]")) then
                if not func.name:match("^[a-z][a-z0-9_]*$") then
                    cclint.report_warning(
                        func.line, 1,
                        string.format("Function name '%s' should use snake_case", func.name)
                    )
                end
            end
        end
    end

    -- Check enum names (should be PascalCase)
    local enums = cclint.get_enums()
    if enums then
        for i, enum in ipairs(enums) do
            if not enum.name:match("^[A-Z][a-zA-Z0-9]*$") then
                cclint.report_warning(
                    enum.line, 1,
                    string.format("Enum name '%s' should use PascalCase", enum.name)
                )
            end

            -- Check enum values (should be UPPER_CASE)
            if enum.values then
                for j, value in ipairs(enum.values) do
                    if not value:match("^[A-Z][A-Z0-9_]*$") then
                        cclint.report_warning(
                            enum.line, 1,
                            string.format("Enum value '%s' should use UPPER_CASE", value)
                        )
                    end
                end
            end
        end
    end
end
