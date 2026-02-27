#pragma once

// Bad example: Violates naming conventions
// Violation: Class name should be BadNaming (PascalCase of bad_naming)
class WrongName {
public:
    WrongName();

    // Violation: Public method not in snake_case
    void SetValue(int val);
    int GetValue() const;

private:
    // Violation: Private method not starting with __
    int calculateInternal() const;

    // Violation: Private method not in snake_case
    void UpdateState();

    // Violation: Member variables not ending with _
    int wrongValue;
    int badData;
};
