// Intentional violations: naming rules
// Expected: class_name_pascal_case, enum_name_pascal_case,
//           private_member_trailing_underscore

#pragma once

#include <string>
#include <vector>

// BAD: class name not PascalCase
class user_manager {
public:
    user_manager() = default;

    void add_user(const std::string& name) { users_.push_back(name); }
    std::size_t user_count() const { return users_.size(); }

private:
    std::vector<std::string> users_;
    int count;      // BAD: private member missing trailing _
    bool is_active; // BAD: private member missing trailing _
};

// BAD: enum name not PascalCase
enum log_level {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR
};
