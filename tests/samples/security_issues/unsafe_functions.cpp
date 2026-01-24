#include <cstdio>
#include <cstdlib>
#include <cstring>

// Bad: Using unsafe gets() function
void unsafe_input() {
    char buffer[100];
    // gets(buffer);  // Unsafe - buffer overflow risk
}

// Bad: Using strcpy without bounds checking
void unsafe_string_copy(const char* src) {
    char dest[10];
    strcpy(dest, src);  // Unsafe - buffer overflow risk
}

// Bad: Using sprintf without bounds checking
void unsafe_sprintf(int value) {
    char buffer[10];
    sprintf(buffer, "Value: %d", value);  // Unsafe - buffer overflow risk
}

// Bad: Using system() with user input
void unsafe_system_call(const char* command) {
    char full_command[256];
    sprintf(full_command, "ls %s", command);  // Command injection risk
    system(full_command);  // Unsafe - arbitrary command execution
}

// Bad: Using rand() for security-sensitive operations
int unsafe_random() {
    return rand();  // Unsafe - not cryptographically secure
}

// Bad: Hardcoded credentials
void hardcoded_password() {
    const char* password = "admin123";  // Security issue
    const char* api_key = "sk_test_1234567890";  // Security issue
}

int main() {
    unsafe_input();
    unsafe_string_copy("This is a very long string that will overflow");
    unsafe_sprintf(42);
    unsafe_system_call(".; rm -rf /");  // Dangerous!
    int random = unsafe_random();
    hardcoded_password();
    return 0;
}
