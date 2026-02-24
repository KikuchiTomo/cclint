#include <iostream>

// Function marked as "main-only" by naming convention (starts with "only_")
void only_initialize_system() {
    std::cout << "System initialized" << std::endl;
}

void only_cleanup() {
    std::cout << "Cleanup complete" << std::endl;
}

// Regular helper function
// Violation: This function calls only_initialize_system, which should only be called from main
void helper_function() {
    only_initialize_system();  // Violation!
    std::cout << "Helper called" << std::endl;
}

void another_helper() {
    std::cout << "Another helper called" << std::endl;
}

// Also violates the rule
void bad_function() {
    only_cleanup();  // Violation!
}

int main() {
    // OK: main calls only_initialize_system
    only_initialize_system();

    // This triggers the violation through helper_function
    helper_function();

    another_helper();

    bad_function();

    // OK: main calls only_cleanup
    only_cleanup();

    return 0;
}
