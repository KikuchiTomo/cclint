-- Rule: Filename must match class name
-- Check that the filename matches the class name (snake_case -> PascalCase)
-- Example: class_name_a.cpp should contain class ClassNameA

rule_description = "Filename should match class name (snake_case -> PascalCase)"
rule_category = "naming"

function snake_to_pascal(snake_str)
    local result = ""
    local capitalize_next = true

    for i = 1, #snake_str do
        local c = snake_str:sub(i, i)
        if c == "_" then
            capitalize_next = true
        else
            if capitalize_next then
                result = result .. c:upper()
                capitalize_next = false
            else
                result = result .. c:lower()
            end
        end
    end

    return result
end

function check_ast()
    print("[DEBUG] filename_matches_class: Starting check")

    -- Get current file path
    local file_info = cclint.get_file_info()
    if not file_info or not file_info.path then
        print("[DEBUG] No file info available")
        return
    end

    local file_path = file_info.path
    print("[DEBUG] File path: " .. file_path)

    -- Extract filename without extension
    local filename = string.match(file_path, "([^/\\]+)%.[^%.]+$")
    if not filename then
        print("[DEBUG] Could not extract filename")
        return
    end

    print("[DEBUG] Filename: " .. filename)

    -- Convert snake_case filename to expected PascalCase class name
    local expected_class_name = snake_to_pascal(filename)
    print("[DEBUG] Expected class name: " .. expected_class_name)

    -- Get all class names (get_classes returns array of strings)
    local class_names = cclint.get_classes()
    if not class_names then
        print("[DEBUG] get_classes() returned nil")
        return
    end

    print("[DEBUG] Found " .. #class_names .. " classes")

    -- Check each class
    for i, class_name in ipairs(class_names) do
        print("[DEBUG] Class " .. i .. ": " .. class_name)

        if class_name ~= expected_class_name then
            -- Get class info to get line number
            local class_info = cclint.get_class_info(class_name)
            local line = 1
            if class_info and class_info.line then
                line = class_info.line
            end

            print("[DEBUG] Reporting error for class: " .. class_name)
            cclint.report_error(
                line, 1,
                string.format(
                    "Class name '%s' does not match filename '%s'. Expected class name: '%s'",
                    class_name, filename, expected_class_name
                )
            )
        end
    end

    print("[DEBUG] filename_matches_class: Check complete")
end
