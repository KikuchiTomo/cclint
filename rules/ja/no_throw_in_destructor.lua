-- デストラクタ内での例外送出を禁止する（std::terminateが呼ばれる）
rule_description = "デストラクタ内で例外を送出しない"
rule_category = "restrictions"

function check_ast()
    local classes = cclint.get_classes()
    if not classes then return end

    for _, class_name in ipairs(classes) do
        local methods = cclint.get_methods(class_name)
        if methods then
            for _, method_name in ipairs(methods) do
                -- デストラクタかどうかを確認（~で始まる）
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
                                    string.format("デストラクタ '%s' 内で例外を送出しないでください。std::terminateが呼び出されます。",
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
