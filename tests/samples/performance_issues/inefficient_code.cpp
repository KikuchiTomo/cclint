#include <iostream>
#include <string>
#include <vector>

// Bad: Pass by value instead of const reference
void process_large_string(std::string str) {  // Unnecessary copy
    std::cout << str << std::endl;
}

// Bad: Not using reserve() for vector
void inefficient_vector_growth() {
    std::vector<int> numbers;
    // Should call numbers.reserve(1000) here
    for (int i = 0; i < 1000; ++i) {
        numbers.push_back(i);  // Multiple reallocations
    }
}

// Bad: Unnecessary string concatenation in loop
std::string inefficient_string_concat(const std::vector<std::string>& words) {
    std::string result;
    for (const auto& word : words) {
        result = result + word + " ";  // Inefficient, creates temporary strings
    }
    return result;
}

// Bad: Using push_back instead of emplace_back
void inefficient_emplace() {
    std::vector<std::pair<int, std::string>> vec;
    vec.push_back(std::make_pair(1, "one"));  // Should use emplace_back
    vec.push_back(std::make_pair(2, "two"));
}

// Bad: Post-increment instead of pre-increment
void inefficient_increment(std::vector<int>& numbers) {
    for (auto it = numbers.begin(); it != numbers.end(); it++) {  // Should be ++it
        std::cout << *it << std::endl;
    }
}

// Bad: Loop-invariant code not hoisted
void loop_invariant_not_hoisted(const std::vector<int>& numbers) {
    for (size_t i = 0; i < numbers.size(); ++i) {
        int threshold = compute_threshold();  // Should be outside loop
        if (numbers[i] > threshold) {
            process(numbers[i]);
        }
    }
}

// Bad: Returning by value from static const string
std::string get_constant_string() {
    return std::string("constant");  // Should be static const
}

// Bad: Using shared_ptr constructor instead of make_shared
void inefficient_shared_ptr() {
    auto ptr = std::shared_ptr<int>(new int(42));  // Should use make_shared
}

int main() {
    std::string large_string = "This is a large string";
    process_large_string(large_string);

    inefficient_vector_growth();

    std::vector<std::string> words = {"hello", "world", "foo", "bar"};
    auto result = inefficient_string_concat(words);

    inefficient_emplace();

    std::vector<int> numbers = {1, 2, 3, 4, 5};
    inefficient_increment(numbers);

    return 0;
}
