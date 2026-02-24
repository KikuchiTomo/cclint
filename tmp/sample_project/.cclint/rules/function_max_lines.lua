-- Rule: Function length must not exceed 1500 lines
-- Check that function definitions are not too long

rule_description = "Functions must not exceed 1500 lines"
rule_category = "readability"

local MAX_LINES = 1500

function check_ast()
    -- Get all functions
    local functions = cclint.get_functions()

    if functions then
        for _, func_name in ipairs(functions) do
            local func_info = cclint.get_function_info(func_name)
            if func_info then
                local line_start = func_info.line or 1
                local line_end = func_info.line_end or func_info.end_line or line_start

                local line_count = line_end - line_start + 1

                if line_count > MAX_LINES then
                    cclint.report_error(
                        line_start, 1,
                        string.format(
                            "Function '%s' is too long (%d lines). Maximum allowed is %d lines. Consider refactoring into smaller functions.",
                            func_name, line_count, MAX_LINES
                        )
                    )
                end
            end
        end
    end

    -- Also check methods in classes
    local classes = cclint.get_classes_with_info()
    if classes then
        for _, class_info in ipairs(classes) do
            local class_name = class_info.name

            if class_name then
                -- Check all methods regardless of access specifier
                for _, access in ipairs({"public", "private", "protected"}) do
                    local methods = cclint.get_class_methods_by_access(class_name, access)

                    if methods then
                        for _, method in ipairs(methods) do
                            local method_name = method.name
                            local line_start = method.line or 1
                            local line_end = method.line_end or method.end_line or line_start

                            local line_count = line_end - line_start + 1

                            if line_count > MAX_LINES then
                                cclint.report_error(
                                    line_start, 1,
                                    string.format(
                                        "Method '%s::%s' is too long (%d lines). Maximum allowed is %d lines. Consider refactoring into smaller functions.",
                                        class_name, method_name, line_count, MAX_LINES
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
