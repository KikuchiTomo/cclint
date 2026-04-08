-- Prohibit std::cout in class methods (use logger instead)
rule_description = "Do not use std::cout in class methods"
rule_category = "restrictions"

function check_ast()
    local classes = cclint.get_classes()
    if not classes then return end

    for _, class_name in ipairs(classes) do
        local methods = cclint.get_methods(class_name)
        if methods then
            for _, method_name in ipairs(methods) do
                local method_info = cclint.get_method_info(class_name, method_name)
                if method_info then
                    -- Check lines around the method for std::cout usage
                    -- Simple heuristic: check next 50 lines from method declaration
                    local start_line = method_info.line
                    local end_line = math.min(start_line + 50, #file_lines)

                    for line_num = start_line, end_line do
                        local line = file_lines[line_num]
                        if line and line:match("std::cout") then
                            cclint.report_warning(
                                line_num, 1,
                                string.format("Do not use std::cout in class '%s' method '%s'. Use a logger instead.",
                                    class_name, method_name)
                            )
                        end
                    end
                end
            end
        end
    end
end
