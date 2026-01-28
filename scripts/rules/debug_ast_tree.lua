-- Debug script to dump entire AST tree
rule_description = "Debug AST tree dump"
rule_category = "debug"

function check_ast()
    print("=== AST Tree Dump ===")

    -- Get all available node types
    local functions = cclint.get_functions()
    local classes = cclint.get_classes()
    local enums = cclint.get_enums()

    print("\n=== Classes (" .. #classes .. ") ===")
    for i, class_name in ipairs(classes) do
        print("  [" .. i .. "] Class: " .. class_name)

        -- Get class info and methods
        local class_info = cclint.get_class_info(class_name)
        if class_info then
            print("    Line: " .. (class_info.line or "?"))
            print("    Is struct: " .. tostring(class_info.is_struct or false))
        end

        -- Get methods for this class
        local methods = cclint.get_methods(class_name)
        if methods then
            local method_count = 0
            for _ in pairs(methods) do method_count = method_count + 1 end
            print("    Methods: " .. method_count)
            for j, method_name in pairs(methods) do
                -- get_methods returns simple array of names
                print("      [" .. j .. "] " .. method_name)

                -- Try to get detailed info for each method
                local method_info = cclint.get_method_info(class_name, method_name)
                if method_info then
                    local access_str = ""
                    if method_info.access == 0 then access_str = "public"
                    elseif method_info.access == 1 then access_str = "protected"
                    elseif method_info.access == 2 then access_str = "private"
                    end
                    print("          " .. access_str .. " " .. (method_info.return_type or "") ..
                          " (line " .. (method_info.line or "?") .. ")")
                end
            end
        end
    end

    print("\n=== Functions (" .. #functions .. ") ===")
    for i, func in ipairs(functions) do
        print("  [" .. i .. "] " .. (func.return_type or "?") .. " " ..
              (func.name or "(no name)") .. " (line " .. (func.line or "?") .. ")")
    end

    print("\n=== Enums (" .. #enums .. ") ===")
    for i, enum in ipairs(enums) do
        print("  [" .. i .. "] " .. (enum.name or "?") .. " (line " .. (enum.line or "?") .. ")")
    end

    print("\n=== End of AST Dump ===")
end
