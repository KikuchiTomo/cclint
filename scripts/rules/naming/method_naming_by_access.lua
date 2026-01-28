-- method_naming_by_access.lua
-- アクセス指定子に基づいてメソッド命名規則をチェックする例

rule_description = "Check method naming conventions based on access specifiers"
rule_category = "naming"

-- 命名規則パターン（正規表現）
local public_pattern = "^[a-z][a-z0-9_]*$"  -- camel_case (例: get_value, process_data)
local protected_pattern = "^[a-z][a-z0-9_]*$"  -- snake_case (例: do_something, helper_func)
local private_pattern = "^_[a-z][a-z0-9_]*$"   -- _snake_case (例: _internal_method, _helper)

-- AST解析関数
function check_ast(file_path)
    -- デバッグ: 関数が呼ばれたことを確認
    print("[DEBUG] check_ast called for: " .. file_path)

    -- すべてのクラスを取得
    local classes = cclint.get_classes()

    if not classes then
        print("[DEBUG] No classes returned from cclint.get_classes()")
        return
    end

    print("[DEBUG] Found " .. #classes .. " class(es)")

    -- 各クラスをチェック
    for i = 1, #classes do
        local class_name = classes[i]
        print("[DEBUG] Checking class: " .. class_name)
        check_class_methods(file_path, class_name)
    end
end

-- クラス内のメソッドをチェック
function check_class_methods(file_path, class_name)
    -- クラスのすべてのメソッドを取得
    local methods = cclint.get_methods(class_name)

    if not methods then
        print("[DEBUG] No methods found for class: " .. class_name)
        return
    end

    print("[DEBUG] Found " .. #methods .. " method(s) in class " .. class_name)

    -- 各メソッドをチェック
    for i = 1, #methods do
        local method_name = methods[i]
        print("[DEBUG] Method " .. i .. ": " .. method_name)
        local info = cclint.get_method_info(class_name, method_name)

        if info then
            print("[DEBUG] Method info - access: " .. (info.access or "none") .. ", line: " .. (info.line or 0))
            check_method_naming(file_path, class_name, method_name, info)
        else
            print("[DEBUG] No info for method: " .. method_name)
        end
    end
end

-- メソッド名が命名規則に従っているかチェック
function check_method_naming(file_path, class_name, method_name, info)
    -- コンストラクタ/デストラクタはスキップ
    if method_name == class_name or method_name == "~" .. class_name then
        print("[DEBUG] Skipping constructor/destructor: " .. method_name)
        return
    end

    local access = info.access
    local line = info.line or 0

    print("[DEBUG] Checking method: " .. method_name .. " (access: " .. access .. ")")

    if access == "public" then
        -- publicメソッドはcamelCaseであるべき
        local matches = string.match(method_name, public_pattern)
        print("[DEBUG] Public pattern match result: " .. tostring(matches ~= nil))
        if not matches then
            print("[DEBUG] Reporting warning for public method: " .. method_name)
            cclint.report_warning(
                line, 0,
                string.format(
                    "Public method '%s::%s' should use snake_case (e.g., get_value, process_data)",
                    class_name, method_name
                )
            )
        end
    elseif access == "protected" then
        -- protectedメソッドはsnake_caseであるべき
        if not string.match(method_name, protected_pattern) then
            cclint.report_warning(
                line, 0,
                string.format(
                    "Protected method '%s::%s' should use snake_case (e.g., do_something, helper_func)",
                    class_name, method_name
                )
            )
        end
    elseif access == "private" then
        -- privateメソッドはアンダースコアで始まるsnake_caseであるべき
        local matches = string.match(method_name, private_pattern)
        print("[DEBUG] Private pattern match result: " .. tostring(matches ~= nil))
        if not matches then
            print("[DEBUG] Reporting warning for private method: " .. method_name)
            cclint.report_warning(
                line, 0,
                string.format(
                    "Private method '%s::%s' should start with underscore and use snake_case (e.g., _internal_method, _helper)",
                    class_name, method_name
                )
            )
        end
    end
end

-- オプション：静的メソッドやconstメソッドにも追加のチェックを適用
function check_method_modifiers(file_path, class_name, method_name, info)
    local line = info.line or 0

    -- staticメソッドは大文字で始めることを推奨（オプション）
    if info.is_static and not string.match(method_name, "^[A-Z]") then
        cclint.report_info(
            line, 0,
            string.format(
                "Static method '%s::%s' could use PascalCase to indicate static nature",
                class_name, method_name
            )
        )
    end

    -- virtualメソッドはoverride/finalの使用を推奨（C++11+）
    if info.is_virtual then
        -- この情報はパーサーの拡張が必要
        -- 今はログ出力のみ
    end
end
