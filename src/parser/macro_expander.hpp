#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "token_types_enhanced.hpp"

namespace cclint {
namespace parser {

// Forward declaration
struct MacroDefinition;

/// Macro expansion engine
/// Handles object-like macros, function-like macros, stringification,
/// token pasting, and variadic macros
class MacroExpander {
public:
    /// Constructor
    /// @param macros Reference to macro definition map
    explicit MacroExpander(const std::unordered_map<std::string, MacroDefinition>& macros);

    /// Expand macros in a token sequence
    /// @param tokens Input tokens
    /// @return Expanded tokens
    std::vector<Token> expand(const std::vector<Token>& tokens);

    /// Expand a single macro invocation
    /// @param macro_token The identifier token of the macro
    /// @param tokens Remaining tokens (for reading arguments)
    /// @param index Current position in tokens (will be updated)
    /// @return Expanded tokens
    std::vector<Token> expand_macro(const Token& macro_token, const std::vector<Token>& tokens,
                                    size_t& index);

    /// Get expansion errors
    const std::vector<std::string>& errors() const { return errors_; }

    /// Check if there were any errors
    bool has_errors() const { return !errors_.empty(); }

private:
    // Macro definitions
    const std::unordered_map<std::string, MacroDefinition>& macros_;

    // Expansion context (to prevent infinite recursion)
    std::unordered_set<std::string> currently_expanding_;

    // Error reporting
    std::vector<std::string> errors_;

    // Object-like macro expansion
    std::vector<Token> expand_object_like_macro(const MacroDefinition& macro,
                                                const Token& macro_token);

    // Function-like macro expansion
    std::vector<Token> expand_function_like_macro(const MacroDefinition& macro,
                                                  const Token& macro_token,
                                                  const std::vector<Token>& tokens, size_t& index);

    // Argument parsing for function-like macros
    std::vector<std::vector<Token>> parse_macro_arguments(const MacroDefinition& macro,
                                                          const std::vector<Token>& tokens,
                                                          size_t& index);

    // Macro replacement with argument substitution
    std::vector<Token> substitute_arguments(const MacroDefinition& macro,
                                            const std::vector<std::vector<Token>>& arguments);

    // Stringification operator (#)
    Token stringify(const std::vector<Token>& tokens);

    // Token pasting operator (##)
    std::vector<Token> paste_tokens(const std::vector<Token>& tokens);
    Token paste_two_tokens(const Token& left, const Token& right);

    // Variadic macro support
    std::vector<Token> handle_va_args(const std::vector<std::vector<Token>>& arguments,
                                      size_t va_args_start);

    // Recursively expand tokens
    std::vector<Token> expand_recursive(const std::vector<Token>& tokens,
                                        const std::string& exclude_macro = "");

    // Helper functions
    bool is_macro_defined(const std::string& name) const;
    const MacroDefinition* get_macro(const std::string& name) const;
    bool is_whitespace_token(const Token& token) const;
    void skip_whitespace(const std::vector<Token>& tokens, size_t& index) const;

    // Error reporting
    void error(const std::string& message);
    void error(const Token& token, const std::string& message);
};

}  // namespace parser
}  // namespace cclint
