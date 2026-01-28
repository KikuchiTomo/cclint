-- Public methods must use snake_case (e.g., get_value, set_data)
rule_description = "Public methods must use snake_case"
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

            -- Check only public methods (access == 0)
            if method_info.access == 0 then
                -- Skip constructors and destructors
                if method_name == class_name or method_name:match("^~") then
                    goto next_method
                end

                -- snake_case: lowercase with underscores, no leading underscore
                if not method_name:match("^[a-z][a-z0-9_]*$") then
                    cclint.report_warning(
                        method_info.line, 1,
                        string.format("Public method '%s' should use snake_case (e.g., get_value)", method_name)
                    )
                end
            end

            ::next_method::
        end

        ::continue::
    end
end
