-- Prohibit 'new' keyword in class methods (use smart pointers instead)
rule_description = "Do not use 'new' in class methods"
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
                    local start_line = method_info.line
                    local end_line = math.min(start_line + 50, #file_lines)

                    for line_num = start_line, end_line do
                        local line = file_lines[line_num]
                        if line and line:match("[^%w_]new%s+[%w_]") then
                            cclint.report_warning(
                                line_num, 1,
                                string.format("Do not use 'new' in class '%s' method '%s'. Use std::make_unique or std::make_shared instead.",
                                    class_name, method_name)
                            )
                        end
                    end
                end
            end
        end
    end
end
