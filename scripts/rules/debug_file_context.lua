-- Debug script to check file context
rule_description = "Debug file context"
rule_category = "debug"

function check_file()
    print("=== File Context Debug ===")
    print("file_path type: " .. type(file_path))
    print("file_path value: " .. tostring(file_path))
    print("file_lines type: " .. type(file_lines))

    if file_lines then
        print("file_lines length: " .. #file_lines)
        print("First 5 lines:")
        for i = 1, math.min(5, #file_lines) do
            print("  Line " .. i .. " (" .. #file_lines[i] .. " chars): " .. file_lines[i])
        end
    else
        print("ERROR: file_lines is nil!")
    end

    print("rule_params type: " .. type(rule_params))
    if rule_params then
        print("rule_params:")
        for k, v in pairs(rule_params) do
            print("  " .. k .. " = " .. tostring(v))
        end
    else
        print("No rule_params")
    end

    print("=== End Debug ===")
end
