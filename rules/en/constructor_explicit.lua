-- Test: Constructor API - single-arg constructors must be explicit
rule_description = "Single-argument constructors must be explicit"
rule_category = "test"

function check_ast()
    local constructors = cclint.get_constructors()

    if not constructors then
        print("[TEST] get_constructors() returned nil")
        return
    end

    print("[TEST] Found " .. #constructors .. " constructors")

    for _, ctor in ipairs(constructors) do
        print("[TEST] Constructor in class: " .. (ctor.class_name or "unknown"))
        print("[TEST]   is_explicit: " .. tostring(ctor.is_explicit))
        print("[TEST]   is_default: " .. tostring(ctor.is_default))
        print("[TEST]   is_delete: " .. tostring(ctor.is_delete))

        -- Check if it's a single-argument constructor
        local info = cclint.get_constructor_info(ctor.class_name)
        if info then
            -- Simple heuristic: if not default and not delete, check explicit
            if not info.is_default and not info.is_delete then
                if not info.is_explicit then
                    cclint.report_warning(
                        ctor.line, 1,
                        "Single-argument constructor should be explicit in class '" .. ctor.class_name .. "'"
                    )
                end
            end
        end
    end
end
