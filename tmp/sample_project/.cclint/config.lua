-- cclint configuration file

-- Load all custom rules
load_rule("rules/one_class_per_file.lua")
load_rule("rules/filename_matches_class.lua")
load_rule("rules/called_only_from_main.lua")
load_rule("rules/private_method_prefix.lua")
load_rule("rules/private_method_snake_case.lua")
load_rule("rules/public_method_snake_case.lua")
load_rule("rules/member_variable_snake_case.lua")
load_rule("rules/member_variable_suffix.lua")
load_rule("rules/function_max_lines.lua")
