-- Prohibit throwing exceptions in destructors (causes std::terminate)
rule_description = "Do not throw exceptions in destructors"
rule_category = "restrictions"

function check_ast()
    local classes = cclint.get_classes()
    if not classes then return end

    for _, class_name in ipairs(classes) do
        local methods = cclint.get_methods(class_name)
        if methods then
            for _, method_name in ipairs(methods) do
                -- Check if this is a destructor (starts with ~)
                if method_name:match("^~") then
                    local method_info = cclint.get_method_info(class_name, method_name)
                    if method_info then
                        local start_line = method_info.line
                        local end_line = math.min(start_line + 50, #file_lines)

                        for line_num = start_line, end_line do
                            local line = file_lines[line_num]
                            if line and line:match("[^%w_]throw%s+") then
                                cclint.report_error(
                                    line_num, 1,
                                    string.format("Do not throw exceptions in destructor '%s'. This will call std::terminate.",
                                        method_name)
                                )
                            end
                        end
                    end
                end
            end
        end
    end
end
