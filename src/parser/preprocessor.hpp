#pragma once

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "enhanced_lexer.hpp"
#include "token_types_enhanced.hpp"

namespace cclint {
namespace parser {

// Forward declaration
class MacroExpander;

/// Represents a preprocessor macro definition
struct MacroDefinition {
    std::string name;
    bool is_function_like;
    std::vector<std::string> parameters;
    bool is_variadic;
    std::vector<Token> replacement_tokens;
    std::string filename;
    int line;

    MacroDefinition() : is_function_like(false), is_variadic(false), line(0) {}
};

/// Represents a conditional compilation context
struct ConditionalContext {
    enum class Type { If, Ifdef, Ifndef, Elif, Else };

    Type type;
    bool condition_result;  // Result of the condition evaluation
    bool has_else;          // Whether an #else has been seen
    bool any_branch_taken;  // Whether any branch has been taken
    int line;               // Line where the directive started

    ConditionalContext(Type t, bool result, int l)
        : type(t), condition_result(result), has_else(false), any_branch_taken(result), line(l) {}
};

/// C/C++ Preprocessor
/// Handles macro expansion, conditional compilation, and file inclusion
class Preprocessor {
public:
    /// Constructor
    /// @param source Source code to preprocess
    /// @param filename Source filename
    /// @param include_paths Additional include search paths
    explicit Preprocessor(const std::string& source, const std::string& filename = "",
                          const std::vector<std::string>& include_paths = {});

    /// Destructor (defined in .cpp to allow unique_ptr with forward declaration)
    ~Preprocessor();

    /// Process the source code and return preprocessed tokens
    /// @return Vector of tokens after preprocessing
    std::vector<Token> preprocess();

    /// Get preprocessing errors
    const std::vector<std::string>& errors() const { return errors_; }

    /// Check if there were any errors
    bool has_errors() const { return !errors_.empty(); }

    /// Define a macro from command line (-D option)
    /// @param definition Macro definition string (e.g., "FOO=1" or "BAR")
    void define_macro(const std::string& definition);

    /// Undefine a macro from command line (-U option)
    /// @param name Macro name to undefine
    void undefine_macro(const std::string& name);

    /// Add an include search path
    void add_include_path(const std::string& path);

    /// Set whether to expand macros (default: false for linter mode)
    /// @param expand If true, macros will be expanded
    void set_expand_macros(bool expand);

    /// Set whether to process includes (default: false for linter mode)
    /// @param expand If true, #include directives will be processed
    void set_expand_includes(bool expand) { expand_includes_ = expand; }

    /// Set whether to expand system includes (default: false)
    /// Only applies if expand_includes is true
    /// @param expand If true, system includes (<>) will be expanded
    void set_expand_system_includes(bool expand) { expand_system_includes_ = expand; }

    /// Get all defined macros (for debugging)
    const std::unordered_map<std::string, MacroDefinition>& macros() const { return macros_; }

private:
    // Source and tokens
    std::string source_;
    std::string filename_;
    std::vector<Token> tokens_;
    size_t current_index_;

    // Macros and include paths
    std::unordered_map<std::string, MacroDefinition> macros_;
    std::vector<std::string> include_paths_;
    std::unordered_set<std::string> included_files_;  // Guard against multiple inclusion

    // Conditional compilation stack
    std::vector<ConditionalContext> conditional_stack_;

    // Macro expander
    std::unique_ptr<MacroExpander> macro_expander_;

    // Preprocessor options (for linter integration)
    bool expand_macros_;                  // Default: false (preserve macro names for rule checking)
    bool expand_includes_;                // Default: false (don't expand includes)
    bool expand_system_includes_;         // Default: false (skip system headers)
    bool predefined_macros_initialized_;  // Track if predefined macros have been defined

    // Error reporting
    std::vector<std::string> errors_;

    // Token access
    const Token& current() const;
    const Token& peek(int offset = 1) const;
    void advance(int count = 1);
    bool is_eof() const;

    // Main preprocessing loop
    void process_tokens();

    // Directive handlers
    void process_directive();
    void process_include();
    void process_define();
    void process_undef();
    void process_if();
    void process_ifdef();
    void process_ifndef();
    void process_elif();
    void process_else();
    void process_endif();
    void process_pragma();
    void process_error();
    void process_warning();
    void process_line();

    // Conditional compilation helpers
    bool evaluate_condition(const std::vector<Token>& tokens);
    bool should_skip_tokens() const;
    void skip_to_endif();
    void skip_to_next_branch();

    // Include file resolution
    std::string resolve_include_path(const std::string& filename, bool is_system);
    std::vector<Token> read_and_lex_file(const std::string& filepath);

    // Macro definition parsing
    MacroDefinition parse_macro_definition();
    std::vector<Token> read_until_newline();

    // Predefined macros
    void define_predefined_macros();
    void update_line_macro();
    void update_file_macro();

    // Error reporting
    void error(const std::string& message);
    void error(const Token& token, const std::string& message);
    void warning(const std::string& message);
    void warning(const Token& token, const std::string& message);
};

}  // namespace parser
}  // namespace cclint
