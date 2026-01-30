#include "preprocessor.hpp"

#include <algorithm>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <sstream>

#include "macro_expander.hpp"

namespace cclint {
namespace parser {

Preprocessor::Preprocessor(const std::string& source, const std::string& filename,
                           const std::vector<std::string>& include_paths)
    : source_(source),
      filename_(filename.empty() ? "<stdin>" : filename),
      current_index_(0),
      include_paths_(include_paths),
      expand_macros_(false),            // Default: don't expand (preserve macro names)
      expand_includes_(false),          // Default: don't expand includes
      expand_system_includes_(false) {  // Default: skip system headers

    // Define predefined macros
    define_predefined_macros();

    // Create macro expander
    macro_expander_ = std::make_unique<MacroExpander>(macros_);
}

Preprocessor::~Preprocessor() = default;

std::vector<Token> Preprocessor::preprocess() {
    // First, tokenize the source
    EnhancedLexer lexer(source_, filename_);
    tokens_ = lexer.tokenize();

    if (lexer.has_errors()) {
        for (const auto& err : lexer.errors()) {
            errors_.push_back(err);
        }
        return {};
    }

    // In linter mode (default), don't process directives, just return tokens
    // The parser will skip directive tokens during parsing
    if (!expand_macros_ && !expand_includes_) {
        // Swap with empty result to transfer ownership without copy
        std::vector<Token> result;
        result.swap(tokens_);
        return result;
    }

    // Process tokens
    current_index_ = 0;
    std::vector<Token> result;

    while (!is_eof()) {
        const auto& token = current();

        // Check if we should skip tokens (inside false conditional branch)
        if (should_skip_tokens()) {
            // Only process directives that affect conditional compilation
            if (token.type == TokenType::PPIfdef || token.type == TokenType::PPIfndef ||
                token.type == TokenType::PPIf || token.type == TokenType::PPElif ||
                token.type == TokenType::PPElse || token.type == TokenType::PPEndif) {
                process_directive();
            } else {
                advance();
            }
            continue;
        }

        // Process preprocessor directives
        if (token.type >= TokenType::PPInclude && token.type <= TokenType::PPLine) {
            // In linter mode (default), skip most directives except conditional compilation
            if (!expand_macros_ && !expand_includes_) {
                // Skip directive in linter mode (just advance past it)
                advance();
            } else {
                process_directive();
            }
        } else if (token.type == TokenType::Eof) {
            result.push_back(token);
            break;
        } else {
            result.push_back(token);
            advance();
        }
    }

    // Check for unclosed conditionals
    if (!conditional_stack_.empty()) {
        error("Unterminated #if/#ifdef/#ifndef directive");
    }

    // Expand macros in the result (if enabled)
    if (expand_macros_ && macro_expander_) {
        auto expanded = macro_expander_->expand(result);

        if (macro_expander_->has_errors()) {
            for (const auto& err : macro_expander_->errors()) {
                errors_.push_back(err);
            }
        }

        return expanded;
    }

    // Return tokens without macro expansion (preserve macro names for rule checking)
    return result;
}

void Preprocessor::define_macro(const std::string& definition) {
    // Parse definition like "FOO=1" or "BAR"
    size_t eq_pos = definition.find('=');

    MacroDefinition macro;
    if (eq_pos != std::string::npos) {
        macro.name = definition.substr(0, eq_pos);
        std::string value = definition.substr(eq_pos + 1);

        // Tokenize the value
        EnhancedLexer lexer(value, "<command-line>");
        macro.replacement_tokens = lexer.tokenize();

        // Remove EOF token
        if (!macro.replacement_tokens.empty() &&
            macro.replacement_tokens.back().type == TokenType::Eof) {
            macro.replacement_tokens.pop_back();
        }
    } else {
        macro.name = definition;
        // Empty replacement (defined but no value)
    }

    macro.is_function_like = false;
    macro.is_variadic = false;
    macro.filename = "<command-line>";
    macro.line = 0;

    macros_[macro.name] = macro;
}

void Preprocessor::undefine_macro(const std::string& name) {
    macros_.erase(name);
}

void Preprocessor::add_include_path(const std::string& path) {
    include_paths_.push_back(path);
}

const Token& Preprocessor::current() const {
    if (current_index_ < tokens_.size()) {
        return tokens_[current_index_];
    }
    static Token eof_token;
    eof_token.type = TokenType::Eof;
    return eof_token;
}

const Token& Preprocessor::peek(int offset) const {
    size_t index = current_index_ + offset;
    if (index < tokens_.size()) {
        return tokens_[index];
    }
    static Token eof_token;
    eof_token.type = TokenType::Eof;
    return eof_token;
}

void Preprocessor::advance(int count) {
    current_index_ += count;
}

bool Preprocessor::is_eof() const {
    return current_index_ >= tokens_.size() || current().type == TokenType::Eof;
}

void Preprocessor::process_directive() {
    const auto& token = current();

    switch (token.type) {
        case TokenType::PPInclude:
            process_include();
            break;
        case TokenType::PPDefine:
            process_define();
            break;
        case TokenType::PPUndef:
            process_undef();
            break;
        case TokenType::PPIf:
            process_if();
            break;
        case TokenType::PPIfdef:
            process_ifdef();
            break;
        case TokenType::PPIfndef:
            process_ifndef();
            break;
        case TokenType::PPElif:
            process_elif();
            break;
        case TokenType::PPElse:
            process_else();
            break;
        case TokenType::PPEndif:
            process_endif();
            break;
        case TokenType::PPPragma:
            process_pragma();
            break;
        case TokenType::PPError:
            process_error();
            break;
        case TokenType::PPWarning:
            process_warning();
            break;
        case TokenType::PPLine:
            process_line();
            break;
        default:
            advance();
            break;
    }
}

void Preprocessor::process_include() {
    // Check if we should expand includes (check early to avoid unnecessary processing)
    if (!expand_includes_) {
        // Don't expand includes in linter mode (skip directive)
        advance();  // Skip #include token
        return;
    }

    // Copy token before advancing
    Token directive_token = current();
    advance();  // Skip #include

    // Parse the directive text (e.g., "#include <iostream>" or "#include \"file.h\"")
    std::string directive_text = directive_token.text;

    // Skip "#include" part
    size_t pos = directive_text.find("include");
    if (pos == std::string::npos) {
        error("Invalid #include directive");
        return;
    }
    pos += 7;  // Skip "include"

    // Skip whitespace
    while (pos < directive_text.size() && std::isspace(directive_text[pos])) {
        ++pos;
    }

    // Determine if it's <> or ""
    bool is_system = false;
    std::string filename;

    if (pos < directive_text.size() && directive_text[pos] == '<') {
        // System include <filename>
        is_system = true;
        ++pos;  // Skip <

        size_t end_pos = directive_text.find('>', pos);
        if (end_pos == std::string::npos) {
            error("Expected '>' in #include");
            return;
        }

        filename = directive_text.substr(pos, end_pos - pos);
    } else if (pos < directive_text.size() && directive_text[pos] == '"') {
        // User include "filename"
        is_system = false;
        ++pos;  // Skip "

        size_t end_pos = directive_text.find('"', pos);
        if (end_pos == std::string::npos) {
            error("Expected '\"' in #include");
            return;
        }

        filename = directive_text.substr(pos, end_pos - pos);
    } else {
        error("Invalid #include directive");
        return;
    }

    // Check if we should skip system includes
    if (is_system && !expand_system_includes_) {
        // Skip system includes to avoid expanding standard library
        return;
    }

    // Resolve the include path
    std::string filepath = resolve_include_path(filename, is_system);

    if (filepath.empty()) {
        warning("Cannot find include file: " + filename);
        return;
    }

    // Check if already included
    if (included_files_.find(filepath) != included_files_.end()) {
        // Already included, skip
        return;
    }

    // Mark as included
    included_files_.insert(filepath);

    // Read and lex the file
    auto included_tokens = read_and_lex_file(filepath);

    // Insert the tokens at the current position
    tokens_.insert(tokens_.begin() + current_index_, included_tokens.begin(),
                   included_tokens.end());
}

void Preprocessor::process_define() {
    // Copy token before advancing
    Token directive_token = current();
    advance();  // Skip #define

    // Parse the directive text (e.g., "#define MAX 100")
    std::string directive_text = directive_token.text;

    // Skip "#define" part
    size_t pos = directive_text.find("define");
    if (pos == std::string::npos) {
        error("Invalid #define directive");
        return;
    }
    pos += 6;  // Skip "define"

    // Skip whitespace
    while (pos < directive_text.size() && std::isspace(directive_text[pos])) {
        ++pos;
    }

    if (pos >= directive_text.size()) {
        error("Expected identifier after #define");
        return;
    }

    // Extract macro name
    MacroDefinition macro;
    while (pos < directive_text.size() &&
           (std::isalnum(directive_text[pos]) || directive_text[pos] == '_')) {
        macro.name += directive_text[pos++];
    }

    if (macro.name.empty()) {
        error("Expected identifier after #define");
        return;
    }

    macro.is_function_like = false;
    macro.is_variadic = false;
    macro.filename = directive_token.filename;
    macro.line = directive_token.line;

    // Check for function-like macro
    if (pos < directive_text.size() && directive_text[pos] == '(') {
        macro.is_function_like = true;
        ++pos;  // Skip (

        // Parse parameters
        while (pos < directive_text.size() && directive_text[pos] != ')') {
            // Skip whitespace
            while (pos < directive_text.size() && std::isspace(directive_text[pos])) {
                ++pos;
            }

            if (pos >= directive_text.size())
                break;

            if (directive_text.substr(pos, 3) == "...") {
                // Variadic macro
                macro.is_variadic = true;
                macro.parameters.push_back("__VA_ARGS__");
                pos += 3;
                break;
            }

            // Extract parameter name
            std::string param;
            while (pos < directive_text.size() &&
                   (std::isalnum(directive_text[pos]) || directive_text[pos] == '_')) {
                param += directive_text[pos++];
            }

            if (!param.empty()) {
                macro.parameters.push_back(param);
            }

            // Skip whitespace
            while (pos < directive_text.size() && std::isspace(directive_text[pos])) {
                ++pos;
            }

            if (pos < directive_text.size() && directive_text[pos] == ',') {
                ++pos;  // Skip comma
            }
        }

        if (pos < directive_text.size() && directive_text[pos] == ')') {
            ++pos;  // Skip )
        }
    }

    // Skip whitespace before replacement
    while (pos < directive_text.size() && std::isspace(directive_text[pos])) {
        ++pos;
    }

    // Get replacement text
    std::string replacement = directive_text.substr(pos);

    // Manually tokenize replacement text to handle # and ## operators
    if (!replacement.empty()) {
        size_t i = 0;
        while (i < replacement.size()) {
            // Skip whitespace
            while (i < replacement.size() && std::isspace(replacement[i])) {
                ++i;
            }

            if (i >= replacement.size())
                break;

            Token token;
            token.filename = macro.filename;
            token.line = macro.line;

            // Check for ##
            if (replacement[i] == '#' && i + 1 < replacement.size() && replacement[i + 1] == '#') {
                token.type = TokenType::MacroConcat;
                token.text = "##";
                i += 2;
                macro.replacement_tokens.push_back(token);
                continue;
            }

            // Check for #
            if (replacement[i] == '#') {
                token.type = TokenType::MacroStringify;
                token.text = "#";
                ++i;
                macro.replacement_tokens.push_back(token);
                continue;
            }

            // For other tokens, collect a chunk and use lexer
            std::string chunk;

            // Collect until we hit # or whitespace
            while (i < replacement.size() && !std::isspace(replacement[i]) &&
                   replacement[i] != '#') {
                chunk += replacement[i];
                ++i;
            }

            // Tokenize the chunk
            if (!chunk.empty()) {
                EnhancedLexer lexer(chunk, macro.filename);
                auto chunk_tokens = lexer.tokenize();

                // Remove EOF
                if (!chunk_tokens.empty() && chunk_tokens.back().type == TokenType::Eof) {
                    chunk_tokens.pop_back();
                }

                macro.replacement_tokens.insert(macro.replacement_tokens.end(),
                                                chunk_tokens.begin(), chunk_tokens.end());
            }
        }
    }

    macros_[macro.name] = macro;

    // Recreate macro expander with updated macros
    macro_expander_ = std::make_unique<MacroExpander>(macros_);
}

void Preprocessor::process_undef() {
    Token directive_token = current();
    advance();  // Skip #undef

    // Parse the directive text (e.g., "#undef MAX")
    std::string directive_text = directive_token.text;

    // Skip "#undef" part
    size_t pos = directive_text.find("undef");
    if (pos == std::string::npos) {
        error("Invalid #undef directive");
        return;
    }
    pos += 5;  // Skip "undef"

    // Skip whitespace
    while (pos < directive_text.size() && std::isspace(directive_text[pos])) {
        ++pos;
    }

    // Extract macro name
    std::string name;
    while (pos < directive_text.size() &&
           (std::isalnum(directive_text[pos]) || directive_text[pos] == '_')) {
        name += directive_text[pos++];
    }

    if (name.empty()) {
        error("Expected identifier after #undef");
        return;
    }

    macros_.erase(name);

    // Recreate macro expander with updated macros
    macro_expander_ = std::make_unique<MacroExpander>(macros_);
}

void Preprocessor::process_if() {
    Token directive_token = current();
    advance();  // Skip #if

    // Parse the directive text (e.g., "#if 1")
    std::string directive_text = directive_token.text;

    // Skip "#if" part
    size_t pos = directive_text.find("if");
    if (pos == std::string::npos) {
        error("Invalid #if directive");
        conditional_stack_.emplace_back(ConditionalContext::Type::If, false, directive_token.line);
        return;
    }

    // Make sure it's not "ifdef" or "ifndef"
    if (pos + 2 < directive_text.size()) {
        char next_char = directive_text[pos + 2];
        if (next_char == 'd' || next_char == 'n') {
            // This is ifdef or ifndef, not if
            error("Internal error: process_if called for ifdef/ifndef");
            conditional_stack_.emplace_back(ConditionalContext::Type::If, false,
                                            directive_token.line);
            return;
        }
    }

    pos += 2;  // Skip "if"

    // Skip whitespace
    while (pos < directive_text.size() && std::isspace(directive_text[pos])) {
        ++pos;
    }

    // Get condition text
    std::string condition_text = directive_text.substr(pos);

    // Tokenize and evaluate condition
    std::vector<Token> condition_tokens;
    if (!condition_text.empty()) {
        EnhancedLexer lexer(condition_text, directive_token.filename);
        condition_tokens = lexer.tokenize();

        // Remove EOF token
        if (!condition_tokens.empty() && condition_tokens.back().type == TokenType::Eof) {
            condition_tokens.pop_back();
        }
    }

    // Evaluate the condition
    bool result = evaluate_condition(condition_tokens);

    // Push conditional context
    conditional_stack_.emplace_back(ConditionalContext::Type::If, result, directive_token.line);
}

void Preprocessor::process_ifdef() {
    Token directive_token = current();
    advance();  // Skip #ifdef

    // Parse the directive text (e.g., "#ifdef DEBUG")
    std::string directive_text = directive_token.text;

    // Skip "#ifdef" part
    size_t pos = directive_text.find("ifdef");
    if (pos == std::string::npos) {
        error("Invalid #ifdef directive");
        return;
    }
    pos += 5;  // Skip "ifdef"

    // Skip whitespace
    while (pos < directive_text.size() && std::isspace(directive_text[pos])) {
        ++pos;
    }

    // Extract macro name
    std::string name;
    while (pos < directive_text.size() &&
           (std::isalnum(directive_text[pos]) || directive_text[pos] == '_')) {
        name += directive_text[pos++];
    }

    if (name.empty()) {
        error("Expected identifier after #ifdef");
        return;
    }

    // Check if macro is defined
    bool result = macros_.find(name) != macros_.end();

    // Push conditional context
    conditional_stack_.emplace_back(ConditionalContext::Type::Ifdef, result, directive_token.line);
}

void Preprocessor::process_ifndef() {
    Token directive_token = current();
    advance();  // Skip #ifndef

    // Parse the directive text (e.g., "#ifndef RELEASE")
    std::string directive_text = directive_token.text;

    // Skip "#ifndef" part
    size_t pos = directive_text.find("ifndef");
    if (pos == std::string::npos) {
        error("Invalid #ifndef directive");
        return;
    }
    pos += 6;  // Skip "ifndef"

    // Skip whitespace
    while (pos < directive_text.size() && std::isspace(directive_text[pos])) {
        ++pos;
    }

    // Extract macro name
    std::string name;
    while (pos < directive_text.size() &&
           (std::isalnum(directive_text[pos]) || directive_text[pos] == '_')) {
        name += directive_text[pos++];
    }

    if (name.empty()) {
        error("Expected identifier after #ifndef");
        return;
    }

    // Check if macro is NOT defined
    bool result = macros_.find(name) == macros_.end();

    // Push conditional context
    conditional_stack_.emplace_back(ConditionalContext::Type::Ifndef, result, directive_token.line);
}

void Preprocessor::process_elif() {
    if (conditional_stack_.empty()) {
        error("#elif without #if");
        return;
    }

    auto& ctx = conditional_stack_.back();

    if (ctx.has_else) {
        error("#elif after #else");
        return;
    }

    Token directive_token = current();
    advance();  // Skip #elif

    // Parse the directive text (e.g., "#elif VERSION == 2")
    std::string directive_text = directive_token.text;

    // Skip "#elif" part
    size_t pos = directive_text.find("elif");
    if (pos == std::string::npos) {
        error("Invalid #elif directive");
        ctx.condition_result = false;
        return;
    }
    pos += 4;  // Skip "elif"

    // Skip whitespace
    while (pos < directive_text.size() && std::isspace(directive_text[pos])) {
        ++pos;
    }

    // Get condition text
    std::string condition_text = directive_text.substr(pos);

    // Tokenize condition
    std::vector<Token> condition_tokens;
    if (!condition_text.empty()) {
        EnhancedLexer lexer(condition_text, directive_token.filename);
        condition_tokens = lexer.tokenize();

        // Remove EOF token
        if (!condition_tokens.empty() && condition_tokens.back().type == TokenType::Eof) {
            condition_tokens.pop_back();
        }
    }

    // Only evaluate if no previous branch was taken
    if (!ctx.any_branch_taken) {
        bool result = evaluate_condition(condition_tokens);
        ctx.condition_result = result;
        if (result) {
            ctx.any_branch_taken = true;
        }
    } else {
        ctx.condition_result = false;
    }

    ctx.type = ConditionalContext::Type::Elif;
}

void Preprocessor::process_else() {
    if (conditional_stack_.empty()) {
        error("#else without #if");
        return;
    }

    auto& ctx = conditional_stack_.back();

    if (ctx.has_else) {
        error("Multiple #else directives");
        return;
    }

    advance();  // Skip #else

    // Flip the condition if no previous branch was taken
    if (!ctx.any_branch_taken) {
        ctx.condition_result = true;
        ctx.any_branch_taken = true;
    } else {
        ctx.condition_result = false;
    }

    ctx.has_else = true;
    ctx.type = ConditionalContext::Type::Else;
}

void Preprocessor::process_endif() {
    if (conditional_stack_.empty()) {
        error("#endif without #if");
        return;
    }

    conditional_stack_.pop_back();
    advance();  // Skip #endif
}

void Preprocessor::process_pragma() {
    advance();  // Skip #pragma

    // For now, we just ignore pragmas
    // A real implementation would parse and handle specific pragmas
}

void Preprocessor::process_error() {
    Token directive_token = current();
    advance();  // Skip #error

    // Parse the directive text (e.g., "#error This is an error")
    std::string directive_text = directive_token.text;

    // Skip "#error" part
    size_t pos = directive_text.find("error");
    if (pos == std::string::npos) {
        error("Invalid #error directive");
        return;
    }
    pos += 5;  // Skip "error"

    // Skip whitespace
    while (pos < directive_text.size() && std::isspace(directive_text[pos])) {
        ++pos;
    }

    // Get error message
    std::string message = directive_text.substr(pos);

    error("#error: " + message);
}

void Preprocessor::process_warning() {
    Token directive_token = current();
    advance();  // Skip #warning

    // Parse the directive text (e.g., "#warning This is a warning")
    std::string directive_text = directive_token.text;

    // Skip "#warning" part
    size_t pos = directive_text.find("warning");
    if (pos == std::string::npos) {
        error("Invalid #warning directive");
        return;
    }
    pos += 7;  // Skip "warning"

    // Skip whitespace
    while (pos < directive_text.size() && std::isspace(directive_text[pos])) {
        ++pos;
    }

    // Get warning message
    std::string message = directive_text.substr(pos);

    warning("#warning: " + message);
}

void Preprocessor::process_line() {
    advance();  // Skip #line

    // For now, we just ignore #line directives
    // A real implementation would parse and update line/file tracking
}

bool Preprocessor::evaluate_condition(const std::vector<Token>& tokens) {
    // Simplified condition evaluation
    // A real implementation would parse and evaluate constant expressions

    // First, expand macros in the condition (if macro_expander is available)
    std::vector<Token> expanded = tokens;
    if (macro_expander_) {
        expanded = macro_expander_->expand(tokens);
    }

    // Very simple evaluation: just check if there's a non-zero number
    for (const auto& token : expanded) {
        if (token.type == TokenType::IntegerLiteral) {
            return token.text != "0";
        }
        if (token.type == TokenType::Identifier) {
            // Undefined identifiers are treated as 0
            return false;
        }
    }

    // Default to true if we can't evaluate
    return true;
}

bool Preprocessor::should_skip_tokens() const {
    for (const auto& ctx : conditional_stack_) {
        if (!ctx.condition_result) {
            return true;
        }
    }
    return false;
}

std::string Preprocessor::resolve_include_path(const std::string& filename, bool is_system) {
    namespace fs = std::filesystem;

    if (!is_system) {
        // Try relative to current file first
        fs::path current_dir = fs::path(filename_).parent_path();
        fs::path full_path = current_dir / filename;

        if (fs::exists(full_path)) {
            return full_path.string();
        }
    }

    // Try include paths
    for (const auto& include_path : include_paths_) {
        fs::path full_path = fs::path(include_path) / filename;
        if (fs::exists(full_path)) {
            return full_path.string();
        }
    }

    // Try standard system paths
    std::vector<std::string> system_paths = {"/usr/include", "/usr/local/include",
                                             "/usr/include/c++/11",
                                             "/usr/include/x86_64-linux-gnu/c++/11"};

    for (const auto& system_path : system_paths) {
        fs::path full_path = fs::path(system_path) / filename;
        if (fs::exists(full_path)) {
            return full_path.string();
        }
    }

    return "";
}

std::vector<Token> Preprocessor::read_and_lex_file(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file) {
        error("Cannot open file: " + filepath);
        return {};
    }

    std::ostringstream oss;
    oss << file.rdbuf();
    std::string content = oss.str();

    EnhancedLexer lexer(content, filepath);
    auto tokens = lexer.tokenize();

    if (lexer.has_errors()) {
        for (const auto& err : lexer.errors()) {
            errors_.push_back(err);
        }
    }

    return tokens;
}

MacroDefinition Preprocessor::parse_macro_definition() {
    MacroDefinition macro;

    // Read macro name
    if (is_eof() || current().type != TokenType::Identifier) {
        error("Expected identifier after #define");
        return macro;
    }

    macro.name = current().text;
    macro.filename = current().filename;
    macro.line = current().line;
    advance();

    // Check for function-like macro
    if (!is_eof() && current().type == TokenType::LeftParen) {
        macro.is_function_like = true;
        advance();  // Skip (

        // Parse parameters
        while (!is_eof() && current().type != TokenType::RightParen) {
            if (current().type == TokenType::Ellipsis) {
                // Variadic macro
                macro.is_variadic = true;
                macro.parameters.push_back("__VA_ARGS__");
                advance();
                break;
            }

            if (current().type == TokenType::Identifier) {
                macro.parameters.push_back(current().text);
                advance();

                if (current().type == TokenType::Comma) {
                    advance();
                } else if (current().type != TokenType::RightParen) {
                    error("Expected ',' or ')' in macro parameter list");
                    return macro;
                }
            } else {
                error("Expected identifier in macro parameter list");
                return macro;
            }
        }

        if (current().type == TokenType::RightParen) {
            advance();  // Skip )
        } else {
            error("Expected ')' after macro parameters");
            return macro;
        }
    }

    // Read replacement tokens
    macro.replacement_tokens = read_until_newline();

    return macro;
}

std::vector<Token> Preprocessor::read_until_newline() {
    std::vector<Token> result;

    while (!is_eof() && !current().is_at_start_of_line) {
        if (current().type == TokenType::Eof) {
            break;
        }
        result.push_back(current());
        advance();
    }

    return result;
}

void Preprocessor::define_predefined_macros() {
    // Helper lambda to create a macro
    auto create_string_macro = [this](const std::string& name, const std::string& value) {
        MacroDefinition macro;
        macro.name = name;
        macro.is_function_like = false;
        macro.is_variadic = false;
        macro.filename = filename_;
        macro.line = 0;

        Token token(TokenType::StringLiteral, value, 0, 0);
        token.value = value;
        token.filename = filename_;
        macro.replacement_tokens.push_back(token);

        macros_.emplace(name, std::move(macro));
    };

    auto create_int_macro = [this](const std::string& name, const std::string& value) {
        MacroDefinition macro;
        macro.name = name;
        macro.is_function_like = false;
        macro.is_variadic = false;
        macro.filename = filename_;
        macro.line = 0;

        Token token(TokenType::IntegerLiteral, value, 0, 0);
        token.value = value;
        token.filename = filename_;
        macro.replacement_tokens.push_back(token);

        macros_.emplace(name, std::move(macro));
    };

    // Define standard macros
    create_string_macro("__FILE__", "\"" + filename_ + "\"");
    create_int_macro("__LINE__", "1");
    create_string_macro("__DATE__", "\"??? ?? ????\"");
    create_string_macro("__TIME__", "\"??:??:??\"");
    create_int_macro("__cplusplus", "201703L");
    create_int_macro("__STDC_HOSTED__", "1");
}

void Preprocessor::error(const std::string& message) {
    errors_.push_back(filename_ + ": error: " + message);
}

void Preprocessor::error(const Token& token, const std::string& message) {
    std::ostringstream oss;
    oss << token.filename << ":" << token.line << ":" << token.column << ": error: " << message;
    errors_.push_back(oss.str());
}

void Preprocessor::warning(const std::string& message) {
    errors_.push_back(filename_ + ": warning: " + message);
}

void Preprocessor::warning(const Token& token, const std::string& message) {
    std::ostringstream oss;
    oss << token.filename << ":" << token.line << ":" << token.column << ": warning: " << message;
    errors_.push_back(oss.str());
}

}  // namespace parser
}  // namespace cclint
