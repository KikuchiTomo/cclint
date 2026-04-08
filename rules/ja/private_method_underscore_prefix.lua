-- privateメソッドはsnake_caseを使用すること (先頭アンダースコア可)
-- 例: _internal_method, calculate_sum
rule_description = "privateメソッドはsnake_caseを使用すること (先頭アンダースコア可)"
rule_category = "naming"

function check_ast()
    local classes = cclint.get_classes()
    if not classes then return end

    for _, class_name in ipairs(classes) do
        local methods = cclint.get_methods(class_name)
        if not methods then goto continue end

        for _, method_name in ipairs(methods) do
            local method_info = cclint.get_method_info(class_name, method_name)
            if not method_info then goto next_method end

            -- Check only private methods (access == 2)
            if method_info.access == 2 then
                -- Skip constructors and destructors
                if method_name == class_name or method_name:match("^~") then
                    goto next_method
                end

                -- Allow: snake_case or _snake_case
                if not method_name:match("^_?[a-z][a-z0-9_]*$") then
                    cclint.report_warning(
                        method_info.line, 1,
                        string.format("privateメソッド '%s' はsnake_caseにしてください (先頭アンダースコア可、例: _internal_method)", method_name)
                    )
                end
            end

            ::next_method::
        end

        ::continue::
    end
end
