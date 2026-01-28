-- Debug script to test API functions
rule_description = "Debug API test"
rule_category = "debug"

function check_ast()
    print("=== DEBUG: Testing get_functions() ===")

    local functions = cclint.get_functions()
    if not functions then
        print("ERROR: get_functions() returned nil")
        return
    end

    print("Found " .. #functions .. " functions")

    for i, func in ipairs(functions) do
        print("Function " .. i .. ":")
        print("  name: '" .. (func.name or "nil") .. "'")
        print("  return_type: '" .. (func.return_type or "nil") .. "'")
        print("  line: " .. (func.line or "nil"))
        print("  file: '" .. (func.file or "nil") .. "'")
        print("  is_static: " .. tostring(func.is_static))
        print("  is_virtual: " .. tostring(func.is_virtual))
        print("  is_const: " .. tostring(func.is_const))
    end

    print("=== DEBUG: Testing get_classes() ===")
    local classes = cclint.get_classes()
    if classes then
        print("Found " .. #classes .. " classes")
        for i, class_name in ipairs(classes) do
            print("Class " .. i .. ": '" .. class_name .. "'")
        end
    else
        print("ERROR: get_classes() returned nil")
    end

    print("=== DEBUG: Testing get_enums() ===")
    local enums = cclint.get_enums()
    if enums then
        print("Found " .. #enums .. " enums")
        for i, enum in ipairs(enums) do
            print("Enum " .. i .. ": '" .. (enum.name or "nil") .. "'")
        end
    else
        print("ERROR: get_enums() returned nil")
    end
end
