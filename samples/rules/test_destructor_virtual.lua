-- Test: Destructor API - base class destructors should be virtual
rule_description = "Base class destructors should be virtual"
rule_category = "test"

function check_ast()
    local destructors = cclint.get_destructors()

    if not destructors then
        print("[TEST] get_destructors() returned nil")
        return
    end

    print("[TEST] Found " .. #destructors .. " destructors")

    -- Get inheritance tree
    local tree = cclint.get_inheritance_tree()

    if not tree then
        print("[TEST] get_inheritance_tree() returned nil")
        return
    end

    -- Check which classes are base classes (have derived classes)
    local base_classes = {}
    for derived, bases in pairs(tree) do
        for _, base in ipairs(bases) do
            base_classes[base] = true
        end
    end

    for _, dtor in ipairs(destructors) do
        print("[TEST] Destructor in class: " .. (dtor.class_name or "unknown"))
        print("[TEST]   is_virtual: " .. tostring(dtor.is_virtual))

        -- If this is a base class and destructor is not virtual
        if base_classes[dtor.class_name] then
            local info = cclint.get_destructor_info(dtor.class_name)
            if info and not info.is_virtual then
                cclint.report_error(
                    dtor.line, 1,
                    "Base class '" .. dtor.class_name .. "' destructor must be virtual"
                )
            end
        end
    end
end
