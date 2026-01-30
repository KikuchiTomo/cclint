#include "macro_expander.hpp"
#include "preprocessor.hpp"
#include <sstream>
#include <algorithm>

namespace cclint {
namespace parser {

MacroExpander::MacroExpander(
    const std::unordered_map<std::string, MacroDefinition>& macros)
    : macros_(macros) {}

std::vector<Token> MacroExpander::expand(const std::vector<Token>& tokens) {
    std::vector<Token> result;

    for (size_t i = 0; i < tokens.size(); ) {
        const auto& token = tokens[i];

        // Check if this is a macro identifier
        if (token.type == TokenType::Identifier && is_macro_defined(token.text)) {
            // Check if we're already expanding this macro (prevent recursion)
            if (currently_expanding_.find(token.text) != currently_expanding_.end()) {
                // Don't expand, just copy the token
                result.push_back(token);
                ++i;
                continue;
            }

            // Expand the macro
            auto expanded = expand_macro(token, tokens, i);
            result.insert(result.end(), expanded.begin(), expanded.end());
        } else {
            // Not a macro, just copy the token
            result.push_back(token);
            ++i;
        }
    }

    return result;
}

std::vector<Token> MacroExpander::expand_macro(
    const Token& macro_token,
    const std::vector<Token>& tokens,
    size_t& index) {

    const auto* macro = get_macro(macro_token.text);
    if (!macro) {
        ++index;
        return {macro_token};
    }

    // Mark this macro as currently expanding
    currently_expanding_.insert(macro->name);

    std::vector<Token> result;

    if (macro->is_function_like) {
        result = expand_function_like_macro(*macro, macro_token, tokens, index);
    } else {
        result = expand_object_like_macro(*macro, macro_token);
        ++index;
    }

    // Unmark the macro
    currently_expanding_.erase(macro->name);

    return result;
}

std::vector<Token> MacroExpander::expand_object_like_macro(
    const MacroDefinition& macro,
    const Token& macro_token) {

    // Simple replacement with macro's replacement tokens
    std::vector<Token> result = macro.replacement_tokens;

    // Recursively expand the result
    return expand_recursive(result, macro.name);
}

std::vector<Token> MacroExpander::expand_function_like_macro(
    const MacroDefinition& macro,
    const Token& macro_token,
    const std::vector<Token>& tokens,
    size_t& index) {

    // Skip the macro name
    ++index;

    // Skip whitespace
    skip_whitespace(tokens, index);

    // Check for opening parenthesis
    if (index >= tokens.size() || tokens[index].type != TokenType::LeftParen) {
        // Not a macro invocation, treat as identifier
        return {macro_token};
    }

    // Parse arguments
    auto arguments = parse_macro_arguments(macro, tokens, index);

    // Check argument count
    size_t expected_args = macro.parameters.size();
    if (macro.is_variadic) {
        // Variadic macros need at least (parameters - 1) arguments
        expected_args = macro.parameters.size() - 1;
    }

    if (!macro.is_variadic && arguments.size() != macro.parameters.size()) {
        error(macro_token, "Macro '" + macro.name + "' expects " +
              std::to_string(macro.parameters.size()) + " arguments, got " +
              std::to_string(arguments.size()));
        return {macro_token};
    }

    if (macro.is_variadic && arguments.size() < expected_args) {
        error(macro_token, "Macro '" + macro.name + "' expects at least " +
              std::to_string(expected_args) + " arguments, got " +
              std::to_string(arguments.size()));
        return {macro_token};
    }

    // Substitute arguments in replacement tokens
    auto substituted = substitute_arguments(macro, arguments);

    // Recursively expand the result
    return expand_recursive(substituted, macro.name);
}

std::vector<std::vector<Token>> MacroExpander::parse_macro_arguments(
    const MacroDefinition& macro,
    const std::vector<Token>& tokens,
    size_t& index) {

    std::vector<std::vector<Token>> arguments;
    std::vector<Token> current_arg;

    // Skip the opening parenthesis
    ++index;

    int paren_depth = 0;

    while (index < tokens.size()) {
        const auto& token = tokens[index];

        if (token.type == TokenType::LeftParen) {
            ++paren_depth;
            current_arg.push_back(token);
        } else if (token.type == TokenType::RightParen) {
            if (paren_depth == 0) {
                // End of arguments
                if (!current_arg.empty() || !arguments.empty()) {
                    arguments.push_back(current_arg);
                }
                ++index;
                break;
            }
            --paren_depth;
            current_arg.push_back(token);
        } else if (token.type == TokenType::Comma && paren_depth == 0) {
            // Next argument
            arguments.push_back(current_arg);
            current_arg.clear();
        } else {
            current_arg.push_back(token);
        }

        ++index;
    }

    return arguments;
}

std::vector<Token> MacroExpander::substitute_arguments(
    const MacroDefinition& macro,
    const std::vector<std::vector<Token>>& arguments) {

    std::vector<Token> result;

    // Build argument map
    std::unordered_map<std::string, std::vector<Token>> arg_map;
    for (size_t i = 0; i < macro.parameters.size() && i < arguments.size(); ++i) {
        arg_map[macro.parameters[i]] = arguments[i];
    }

    // Handle __VA_ARGS__ for variadic macros
    if (macro.is_variadic && arguments.size() >= macro.parameters.size()) {
        std::vector<Token> va_args;
        for (size_t i = macro.parameters.size() - 1; i < arguments.size(); ++i) {
            if (i > macro.parameters.size() - 1) {
                // Add comma separator
                Token comma;
                comma.type = TokenType::Comma;
                comma.text = ",";
                va_args.push_back(comma);
            }
            va_args.insert(va_args.end(), arguments[i].begin(), arguments[i].end());
        }
        arg_map["__VA_ARGS__"] = va_args;
    }

    // Process replacement tokens
    for (size_t i = 0; i < macro.replacement_tokens.size(); ++i) {
        const auto& token = macro.replacement_tokens[i];

        // Check for stringification (#)
        if (token.type == TokenType::MacroStringify) {
            if (i + 1 < macro.replacement_tokens.size()) {
                const auto& next = macro.replacement_tokens[i + 1];
                if (arg_map.find(next.text) != arg_map.end()) {
                    // Stringify the argument
                    result.push_back(stringify(arg_map[next.text]));
                    ++i;  // Skip the parameter name
                    continue;
                }
            }
        }

        // Check for token pasting (##)
        if (token.type == TokenType::MacroConcat) {
            // Will be handled in paste_tokens
            result.push_back(token);
            continue;
        }

        // Check if this is a macro parameter
        if (token.type == TokenType::Identifier && arg_map.find(token.text) != arg_map.end()) {
            // Substitute with argument tokens (expanded)
            auto arg_tokens = arg_map[token.text];
            auto expanded_arg = expand_recursive(arg_tokens);
            result.insert(result.end(), expanded_arg.begin(), expanded_arg.end());
        } else {
            result.push_back(token);
        }
    }

    // Handle token pasting
    return paste_tokens(result);
}

Token MacroExpander::stringify(const std::vector<Token>& tokens) {
    std::ostringstream oss;
    oss << "\"";

    for (const auto& token : tokens) {
        // Escape special characters in the token text
        for (char c : token.text) {
            if (c == '"' || c == '\\') {
                oss << '\\';
            }
            oss << c;
        }

        // Add space between tokens (except for the last one)
        if (&token != &tokens.back()) {
            oss << " ";
        }
    }

    oss << "\"";

    Token result;
    result.type = TokenType::StringLiteral;
    result.text = oss.str();
    result.value = oss.str();
    return result;
}

std::vector<Token> MacroExpander::paste_tokens(const std::vector<Token>& tokens) {
    std::vector<Token> result;

    for (size_t i = 0; i < tokens.size(); ++i) {
        if (tokens[i].type == TokenType::MacroConcat) {
            if (result.empty() || i + 1 >= tokens.size()) {
                error(tokens[i], "## operator requires tokens on both sides");
                continue;
            }

            // Paste the previous token with the next token
            Token left = result.back();
            result.pop_back();

            Token right = tokens[i + 1];
            ++i;  // Skip the next token

            Token pasted = paste_two_tokens(left, right);
            result.push_back(pasted);
        } else {
            result.push_back(tokens[i]);
        }
    }

    return result;
}

Token MacroExpander::paste_two_tokens(const Token& left, const Token& right) {
    Token result;
    result.text = left.text + right.text;
    result.filename = left.filename;
    result.line = left.line;
    result.column = left.column;

    // Re-lex the pasted token to determine its type
    // For simplicity, we'll keep it as Identifier or the original type
    // A proper implementation would re-lex the concatenated string
    if (left.type == TokenType::Identifier || right.type == TokenType::Identifier) {
        result.type = TokenType::Identifier;
    } else {
        result.type = left.type;
    }

    return result;
}

std::vector<Token> MacroExpander::expand_recursive(
    const std::vector<Token>& tokens,
    const std::string& exclude_macro) {

    std::vector<Token> result;

    for (size_t i = 0; i < tokens.size(); ) {
        const auto& token = tokens[i];

        // Check if this is a macro identifier
        if (token.type == TokenType::Identifier &&
            is_macro_defined(token.text) &&
            token.text != exclude_macro &&
            currently_expanding_.find(token.text) == currently_expanding_.end()) {

            // Expand the macro
            auto expanded = expand_macro(token, tokens, i);
            result.insert(result.end(), expanded.begin(), expanded.end());
        } else {
            // Not a macro, just copy the token
            result.push_back(token);
            ++i;
        }
    }

    return result;
}

bool MacroExpander::is_macro_defined(const std::string& name) const {
    return macros_.find(name) != macros_.end();
}

const MacroDefinition* MacroExpander::get_macro(const std::string& name) const {
    auto it = macros_.find(name);
    if (it != macros_.end()) {
        return &it->second;
    }
    return nullptr;
}

bool MacroExpander::is_whitespace_token(const Token& token) const {
    return token.type == TokenType::Whitespace;
}

void MacroExpander::skip_whitespace(const std::vector<Token>& tokens, size_t& index) const {
    while (index < tokens.size() && is_whitespace_token(tokens[index])) {
        ++index;
    }
}

void MacroExpander::error(const std::string& message) {
    errors_.push_back(message);
}

void MacroExpander::error(const Token& token, const std::string& message) {
    std::ostringstream oss;
    oss << token.filename << ":" << token.line << ":" << token.column
        << ": error: " << message;
    errors_.push_back(oss.str());
}

} // namespace parser
} // namespace cclint
