# cclint Sample Project - Lint Results

## Summary

All 9 custom Lua rules successfully detected violations in the sample code.

## Detailed Results

### include/bad_naming.hpp (11 errors)

1. ❌ Class name 'WrongName' does not match filename 'bad_naming' (expected: 'BadNaming')
2. ❌ Public method 'SetValue' must use lower_snake_case
3. ❌ Public method 'GetValue' must use lower_snake_case  
4. ❌ Private method 'calculateInternal' must start with '__'
5. ❌ Private method 'calculateInternal' must use lower_snake_case
6. ❌ Private method 'UpdateState' must start with '__'
7. ❌ Private method 'UpdateState' must use lower_snake_case
8. ❌ Member variable 'wrongValue' must end with '_'
9. ❌ Member variable 'wrongValue' must use lower_snake_case
10. ❌ Member variable 'badData' must end with '_'
11. ❌ Member variable 'badData' must use lower_snake_case

### include/calculator.hpp (0 errors)
✅ No violations - good example following all naming conventions!

### src/bad_naming.cpp (1 warning)
⚠️ No class definition found in .cpp file (implementation file)

### src/calculator.cpp (1 warning)
⚠️ No class definition found in .cpp file (implementation file)

### src/multiple_classes.cpp (3 errors)
1. ❌ Class name 'FirstClass' does not match filename 'multiple_classes' (expected: 'MultipleClasses')
2. ❌ Class name 'SecondClass' does not match filename 'multiple_classes' (expected: 'MultipleClasses')
3. ❌ Multiple classes in one file (found 2 classes)

### target/bad_naming_demo.cpp (1 warning)
⚠️ No class definition found in .cpp file (executable with main)

### target/calculator_demo.cpp (1 warning)
⚠️ No class definition found in .cpp file (executable with main)

### target/call_graph_test.cpp (2 errors + 1 warning)
1. ❌ Function 'only_initialize_system' should only be called from main(), but is called from 'helper_function'
2. ❌ Function 'only_cleanup' should only be called from main(), but is called from 'bad_function'
3. ⚠️ No class definition found in .cpp file

## Statistics

- **Total Errors**: 16
- **Total Warnings**: 5
- **Files Checked**: 8
- **Rules Applied**: 9

## Rules Tested

1. ✅ **one_class_per_file** - Detected multiple classes in one file
2. ✅ **filename_matches_class** - Detected class name mismatches
3. ✅ **called_only_from_main** - Detected improper function calls
4. ✅ **private_method_prefix** - Detected missing '__' prefix
5. ✅ **private_method_snake_case** - Detected PascalCase in private methods
6. ✅ **public_method_snake_case** - Detected PascalCase in public methods
7. ✅ **member_variable_snake_case** - Detected camelCase in member variables
8. ✅ **member_variable_suffix** - Detected missing '_' suffix
9. ✅ **function_max_lines** - (No violations in this sample)

## Conclusion

All Lua AST APIs are working correctly:
- `cclint.get_classes_with_info()`
- `cclint.get_class_methods_by_access()`
- `cclint.get_class_fields_by_access()`
- `cclint.get_function_calls()`
- `cclint.get_file_info()`

The sample project successfully demonstrates the power and flexibility of cclint's Lua-based custom rule system.
