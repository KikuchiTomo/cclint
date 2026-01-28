#include "parser/builtin_parser.hpp"

namespace cclint {
namespace parser {

BuiltinParser::BuiltinParser(const std::string& source, const std::string& filename)
    : filename_(filename) {
    Lexer lexer(source);
    tokens_ = lexer.tokenize();
}

std::shared_ptr<TranslationUnitNode> BuiltinParser::parse() {
    auto root = std::make_shared<TranslationUnitNode>();
    root->name = filename_;

    while (!check(TokenType::Eof)) {
        parse_toplevel(*root);
    }

    return root;
}

const Token& BuiltinParser::current_token() const {
    if (current_ < tokens_.size()) {
        return tokens_[current_];
    }
    static Token eof_token(TokenType::Eof, "", 0, 0);
    return eof_token;
}

const Token& BuiltinParser::peek_token(int offset) const {
    size_t pos = current_ + offset;
    if (pos < tokens_.size()) {
        return tokens_[pos];
    }
    static Token eof_token(TokenType::Eof, "", 0, 0);
    return eof_token;
}

bool BuiltinParser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool BuiltinParser::check(TokenType type) const {
    return current_token().type == type;
}

Token BuiltinParser::advance() {
    if (current_ < tokens_.size()) {
        return tokens_[current_++];
    }
    return Token(TokenType::Eof, "", 0, 0);
}

Token BuiltinParser::expect(TokenType type, const std::string& message) {
    if (check(type)) {
        return advance();
    }
    add_error(message);
    return current_token();
}

void BuiltinParser::parse_toplevel(TranslationUnitNode& root) {
    // コメントとプリプロセッサをスキップ
    while (match(TokenType::Comment) || match(TokenType::Preprocessor)) {
        // スキップ
    }

    if (check(TokenType::Eof)) {
        return;
    }

    // namespace
    if (check(TokenType::Namespace)) {
        auto node = parse_namespace();
        if (node) {
            root.children.push_back(node);
        }
        return;
    }

    // class/struct
    if (check(TokenType::Class) || check(TokenType::Struct)) {
        auto node = parse_class_or_struct();
        if (node) {
            root.children.push_back(node);
        }
        return;
    }

    // enum
    if (check(TokenType::Enum)) {
        auto node = parse_enum();
        if (node) {
            root.children.push_back(node);
        }
        return;
    }

    // typedef
    if (check(TokenType::Typedef)) {
        auto node = parse_typedef();
        if (node) {
            root.children.push_back(node);
        }
        return;
    }

    // using
    if (check(TokenType::Using)) {
        auto node = parse_using();
        if (node) {
            root.children.push_back(node);
        }
        return;
    }

    // template
    if (check(TokenType::Template)) {
        auto tmpl = std::make_shared<TemplateNode>();
        tmpl->position = get_position();
        advance();  // consume 'template'

        // Parse template parameters <...>
        if (match(TokenType::Less)) {
            int depth = 1;
            while (depth > 0 && !check(TokenType::Eof)) {
                if (match(TokenType::Less))
                    depth++;
                else if (match(TokenType::Greater))
                    depth--;
                else {
                    // Simple parameter extraction
                    if (check(TokenType::Identifier)) {
                        TemplateParameter param;
                        param.kind = TemplateParameter::Kind::Type;
                        param.name = advance().text;
                        tmpl->parameters.push_back(param);
                    } else {
                        advance();
                    }
                }
            }
        }

        // Parse the templated entity (class/function)
        auto entity = parse_function_or_variable();
        if (!entity && (check(TokenType::Class) || check(TokenType::Struct))) {
            entity = parse_class_or_struct();
        }
        if (entity) {
            tmpl->children.push_back(entity);
            root.children.push_back(tmpl);
        }
        return;
    }

    // static_assert
    if (current_token().text == "static_assert") {
        auto sa = std::make_shared<StaticAssertNode>();
        sa->position = get_position();
        advance();  // consume 'static_assert'

        if (match(TokenType::LeftParen)) {
            // Parse condition
            std::string condition;
            int depth = 1;
            while (depth > 0 && !check(TokenType::Eof)) {
                if (match(TokenType::LeftParen))
                    depth++;
                else if (match(TokenType::RightParen)) {
                    depth--;
                    if (depth == 0)
                        break;
                } else {
                    condition += current_token().text + " ";
                    advance();
                }
            }
            sa->condition = condition;
        }
        match(TokenType::Semicolon);
        root.children.push_back(sa);
        return;
    }

    // 関数または変数
    auto node = parse_function_or_variable();
    if (node) {
        root.children.push_back(node);
    }
}

std::shared_ptr<ASTNode> BuiltinParser::parse_namespace() {
    auto node = std::make_shared<NamespaceNode>();
    node->position = get_position();

    expect(TokenType::Namespace, "Expected 'namespace'");

    // namespace name
    if (check(TokenType::Identifier)) {
        node->name = advance().text;
    }

    if (!match(TokenType::LeftBrace)) {
        add_error("Expected '{' after namespace name");
        skip_to_semicolon();
        return node;
    }

    // namespace body - namespace内の要素を直接childrenに追加
    while (!check(TokenType::RightBrace) && !check(TokenType::Eof)) {
        // コメントとプリプロセッサをスキップ
        while (match(TokenType::Comment) || match(TokenType::Preprocessor)) {
        }

        if (check(TokenType::RightBrace) || check(TokenType::Eof)) {
            break;
        }

        // ネストされた宣言をパース
        std::shared_ptr<ASTNode> child;

        if (check(TokenType::Namespace)) {
            child = parse_namespace();
        } else if (check(TokenType::Class) || check(TokenType::Struct)) {
            child = parse_class_or_struct();
        } else if (check(TokenType::Enum)) {
            child = parse_enum();
        } else if (check(TokenType::Typedef)) {
            child = parse_typedef();
        } else if (check(TokenType::Using)) {
            child = parse_using();
        } else if (check(TokenType::Template)) {
            advance();
            skip_to_semicolon();
            continue;
        } else {
            child = parse_function_or_variable();
        }

        if (child) {
            node->children.push_back(child);
        }
    }

    match(TokenType::RightBrace);

    return node;
}

std::shared_ptr<ASTNode> BuiltinParser::parse_class_or_struct() {
    auto node = std::make_shared<ClassNode>();
    node->position = get_position();

    if (match(TokenType::Struct)) {
        node->is_struct = true;
        current_access_ = AccessSpecifier::Public;
    } else if (match(TokenType::Class)) {
        node->is_struct = false;
        current_access_ = AccessSpecifier::Private;
    } else {
        add_error("Expected 'class' or 'struct'");
        return nullptr;
    }

    // class name
    if (check(TokenType::Identifier)) {
        node->name = advance().text;
    } else {
        add_error("Expected class name");
        return nullptr;
    }

    // skip template parameters or inheritance
    if (match(TokenType::Less)) {
        int depth = 1;
        while (depth > 0 && !check(TokenType::Eof)) {
            if (match(TokenType::Less))
                depth++;
            else if (match(TokenType::Greater))
                depth--;
            else
                advance();
        }
    }

    // inheritance
    if (match(TokenType::Colon)) {
        while (!check(TokenType::LeftBrace) && !check(TokenType::Eof)) {
            if (check(TokenType::Identifier)) {
                InheritanceInfo inherit_info;
                inherit_info.base_class_name = advance().text;
                inherit_info.access = AccessSpecifier::Public;  // デフォルトpublic
                inherit_info.is_virtual = false;
                node->base_classes.push_back(inherit_info);
            }
            advance();
        }
    }

    if (!match(TokenType::LeftBrace)) {
        skip_to_semicolon();
        return node;
    }

    // class body
    while (!check(TokenType::RightBrace) && !check(TokenType::Eof)) {
        // アクセス指定子
        if (match(TokenType::Public)) {
            current_access_ = AccessSpecifier::Public;
            expect(TokenType::Colon, "Expected ':' after access specifier");
            continue;
        }
        if (match(TokenType::Protected)) {
            current_access_ = AccessSpecifier::Protected;
            expect(TokenType::Colon, "Expected ':' after access specifier");
            continue;
        }
        if (match(TokenType::Private)) {
            current_access_ = AccessSpecifier::Private;
            expect(TokenType::Colon, "Expected ':' after access specifier");
            continue;
        }

        // コメントをスキップ
        if (match(TokenType::Comment) || match(TokenType::Preprocessor)) {
            continue;
        }

        // Friend宣言をチェック
        if (current_token().text == "friend") {
            advance();  // consume 'friend'
            auto friend_node = std::make_shared<FriendNode>();
            friend_node->position = get_position();

            // friend class or friend function
            if (match(TokenType::Class) || match(TokenType::Struct)) {
                friend_node->kind = FriendNode::FriendKind::Class;
                if (check(TokenType::Identifier)) {
                    friend_node->target_name = advance().text;
                }
            } else {
                friend_node->kind = FriendNode::FriendKind::Function;
                // Parse function signature
                std::string target;
                while (!check(TokenType::Semicolon) && !check(TokenType::Eof)) {
                    target += current_token().text + " ";
                    advance();
                }
                friend_node->target_name = target;
            }
            match(TokenType::Semicolon);
            node->children.push_back(friend_node);
            continue;
        }

        // メンバ関数または変数
        auto member = parse_function_or_variable();
        if (member) {
            if (auto func = std::dynamic_pointer_cast<FunctionNode>(member)) {
                func->access = current_access_;

                // コンストラクタの検出と変換
                if ((func->name.empty() && func->return_type == node->name) ||
                    func->name == node->name) {
                    auto ctor = std::make_shared<ConstructorNode>();
                    ctor->class_name = node->name;
                    ctor->position = func->position;
                    ctor->access = current_access_;
                    ctor->is_explicit = false;  // TODO: detect explicit keyword
                    ctor->is_default = false;
                    ctor->is_delete = false;
                    ctor->is_constexpr = false;
                    ctor->is_noexcept = false;
                    node->children.push_back(ctor);
                    continue;
                }

                // デストラクタの検出と変換
                if (func->name.size() > 0 && func->name[0] == '~') {
                    auto dtor = std::make_shared<DestructorNode>();
                    dtor->class_name = node->name;
                    dtor->position = func->position;
                    dtor->access = current_access_;
                    dtor->is_virtual = func->is_virtual;
                    dtor->is_default = false;
                    dtor->is_delete = false;
                    dtor->is_noexcept = false;
                    node->children.push_back(dtor);
                    continue;
                }

                // operator overloadの検出
                if (func->name.find("operator") == 0) {
                    auto op = std::make_shared<OperatorNode>();
                    op->operator_symbol = func->name.substr(8);  // Remove "operator"
                    op->position = func->position;
                    op->is_member = true;  // In class, so member operator
                    op->return_type = func->return_type;
                    node->children.push_back(op);
                    continue;
                }

            } else if (auto field = std::dynamic_pointer_cast<VariableNode>(member)) {
                auto field_node = std::make_shared<FieldNode>();
                field_node->name = field->name;
                field_node->type_name = field->type_name;
                field_node->is_const = field->is_const;
                field_node->is_static = field->is_static;
                field_node->access = current_access_;
                field_node->position = field->position;
                node->children.push_back(field_node);
                continue;
            }
            node->children.push_back(member);
        }
    }

    match(TokenType::RightBrace);
    match(TokenType::Semicolon);

    return node;
}

std::shared_ptr<ASTNode> BuiltinParser::parse_enum() {
    auto node = std::make_shared<EnumNode>();
    node->position = get_position();

    advance();  // 'enum'

    // enum class?
    if (match(TokenType::Class)) {
        node->is_class = true;
    }

    // enum name
    if (check(TokenType::Identifier)) {
        node->name = advance().text;
    }

    // skip body
    skip_braces();
    match(TokenType::Semicolon);

    return node;
}

std::shared_ptr<ASTNode> BuiltinParser::parse_function_or_variable() {
    SourcePosition pos = get_position();

    bool is_static = false;
    bool is_const = false;
    bool is_virtual = false;
    bool is_constexpr = false;

    // 修飾子
    while (true) {
        if (match(TokenType::Static)) {
            is_static = true;
        } else if (match(TokenType::Virtual)) {
            is_virtual = true;
        } else if (match(TokenType::Constexpr)) {
            is_constexpr = true;
        } else if (match(TokenType::Const) && !is_const) {
            is_const = true;
        } else {
            break;
        }
    }

    // Check for destructor (~ClassName)
    bool is_destructor = false;
    if (current_token().text == "~") {
        is_destructor = true;
        advance();  // consume ~
    }

    // 型
    std::string type_name;
    if (!is_destructor) {
        type_name = parse_type();
    }

    // 名前
    std::string name;
    if (check(TokenType::Identifier)) {
        name = advance().text;
        if (is_destructor) {
            name = "~" + name;
        }

        // Check for operator overload
        if (name == "operator") {
            // Next token should be the operator symbol
            if (!check(TokenType::LeftParen) && !check(TokenType::Eof)) {
                name += current_token().text;
                advance();
            }
        }
    }

    // 関数かどうか判定
    if (match(TokenType::LeftParen)) {
        // 関数
        auto func = std::make_shared<FunctionNode>();
        func->name = name;
        func->return_type = type_name;
        func->is_static = is_static;
        func->is_virtual = is_virtual;
        func->position = pos;

        // パラメータをスキップ
        int depth = 1;
        while (depth > 0 && !check(TokenType::Eof)) {
            if (match(TokenType::LeftParen))
                depth++;
            else if (match(TokenType::RightParen))
                depth--;
            else
                advance();
        }

        // 修飾子 (const, override, final)
        while (true) {
            if (match(TokenType::Const)) {
                func->is_const = true;
            } else if (match(TokenType::Override)) {
                func->is_override = true;
            } else if (match(TokenType::Final)) {
                func->is_final = true;
            } else {
                break;
            }
        }

        // 関数本体またはセミコロン
        if (check(TokenType::LeftBrace)) {
            // Parse function body to detect lambdas and function calls
            advance();  // consume '{'
            int depth = 1;

            std::string current_function = func->name;

            while (depth > 0 && !check(TokenType::Eof)) {
                // Detect lambda: [...](...)
                if (check(TokenType::LeftBracket)) {
                    size_t save_pos = current_;
                    advance();  // consume '['

                    // Check if this looks like a lambda capture
                    bool is_lambda = false;
                    while (!check(TokenType::RightBracket) && !check(TokenType::Eof) &&
                           (current_ - save_pos) < 50) {
                        if (check(TokenType::Identifier) || current_token().text == "=" ||
                            current_token().text == "&" || current_token().text == "," ||
                            current_token().text == "this") {
                            advance();
                        } else if (check(TokenType::RightBracket)) {
                            break;
                        } else {
                            break;
                        }
                    }

                    if (check(TokenType::RightBracket)) {
                        advance();  // consume ']'
                        // Check for (...) after capture
                        if (check(TokenType::LeftParen)) {
                            is_lambda = true;
                        }
                    }

                    if (is_lambda) {
                        auto lambda = std::make_shared<LambdaNode>();
                        lambda->position = get_position();
                        // Simplified lambda parsing - just capture that we found one
                        current_ = save_pos;
                        advance();  // '['

                        std::string capture;
                        while (!check(TokenType::RightBracket) && !check(TokenType::Eof)) {
                            capture += current_token().text;
                            if (current_token().text == "=") {
                                lambda->captures_by_value = true;
                            } else if (current_token().text == "&") {
                                lambda->captures_by_reference = true;
                            }
                            advance();
                        }
                        lambda->capture_clause = "[" + capture + "]";
                        match(TokenType::RightBracket);

                        // Skip parameters
                        if (match(TokenType::LeftParen)) {
                            int pdepth = 1;
                            while (pdepth > 0 && !check(TokenType::Eof)) {
                                if (match(TokenType::LeftParen))
                                    pdepth++;
                                else if (match(TokenType::RightParen))
                                    pdepth--;
                                else
                                    advance();
                            }
                        }

                        // Check for mutable/constexpr
                        if (current_token().text == "mutable") {
                            lambda->is_mutable = true;
                            advance();
                        }
                        if (current_token().text == "constexpr") {
                            lambda->is_constexpr = true;
                            advance();
                        }

                        // Skip body
                        if (check(TokenType::LeftBrace)) {
                            int ldepth = 1;
                            advance();
                            while (ldepth > 0 && !check(TokenType::Eof)) {
                                if (match(TokenType::LeftBrace))
                                    ldepth++;
                                else if (match(TokenType::RightBrace))
                                    ldepth--;
                                else
                                    advance();
                            }
                        }

                        func->children.push_back(lambda);
                        continue;
                    } else {
                        current_ = save_pos;
                    }
                }

                // Detect function calls: identifier(
                if (check(TokenType::Identifier)) {
                    std::string callee_name = current_token().text;
                    advance();

                    if (match(TokenType::LeftParen)) {
                        // This is a function call
                        auto call = std::make_shared<CallExpressionNode>();
                        call->function_name = callee_name;
                        call->caller_function = current_function;
                        call->position = get_position();

                        // Skip arguments
                        int cdepth = 1;
                        while (cdepth > 0 && !check(TokenType::Eof)) {
                            if (match(TokenType::LeftParen))
                                cdepth++;
                            else if (match(TokenType::RightParen))
                                cdepth--;
                            else
                                advance();
                        }

                        func->children.push_back(call);
                        continue;
                    }
                }

                if (match(TokenType::LeftBrace)) {
                    depth++;
                } else if (match(TokenType::RightBrace)) {
                    depth--;
                } else {
                    advance();
                }
            }
        } else {
            match(TokenType::Semicolon);
        }

        return func;
    } else {
        // 変数 - check for lambda initialization
        if (check(TokenType::LeftBracket)) {
            // Potential lambda: auto lambda = [...](...){}
            auto lambda = std::make_shared<LambdaNode>();
            lambda->position = pos;

            // Parse capture clause [...]
            advance();  // consume '['
            std::string capture;
            while (!check(TokenType::RightBracket) && !check(TokenType::Eof)) {
                capture += current_token().text;
                if (current_token().text == "=") {
                    lambda->captures_by_value = true;
                } else if (current_token().text == "&") {
                    lambda->captures_by_reference = true;
                }
                advance();
            }
            lambda->capture_clause = "[" + capture + "]";
            match(TokenType::RightBracket);

            // Parse parameters (...)
            if (match(TokenType::LeftParen)) {
                int depth = 1;
                while (depth > 0 && !check(TokenType::Eof)) {
                    if (match(TokenType::LeftParen))
                        depth++;
                    else if (match(TokenType::RightParen))
                        depth--;
                    else
                        advance();
                }
            }

            // Check for mutable
            if (current_token().text == "mutable") {
                lambda->is_mutable = true;
                advance();
            }

            // Check for constexpr
            if (current_token().text == "constexpr") {
                lambda->is_constexpr = true;
                advance();
            }

            // Skip body {...}
            if (check(TokenType::LeftBrace)) {
                skip_braces();
            }

            match(TokenType::Semicolon);
            return lambda;
        }

        // 変数
        auto var = std::make_shared<VariableNode>();
        var->name = name;
        var->type_name = type_name;
        var->is_static = is_static;
        var->is_const = is_const;
        var->is_constexpr = is_constexpr;
        var->position = pos;

        skip_to_semicolon();
        match(TokenType::Semicolon);

        return var;
    }
}

std::shared_ptr<ASTNode> BuiltinParser::parse_typedef() {
    auto node = std::make_shared<TypedefNode>();
    node->position = get_position();

    advance();  // 'typedef'

    skip_to_semicolon();
    match(TokenType::Semicolon);

    return node;
}

std::shared_ptr<ASTNode> BuiltinParser::parse_using() {
    auto node = std::make_shared<UsingNode>();
    node->position = get_position();

    advance();  // 'using'

    if (check(TokenType::Identifier)) {
        node->name = advance().text;
    }

    skip_to_semicolon();
    match(TokenType::Semicolon);

    return node;
}

std::string BuiltinParser::parse_type() {
    std::string type;
    bool has_base_type = false;  // Track if we've seen the main type

    // Helper lambda to check if string ends with suffix (C++17 compatible)
    auto ends_with = [](const std::string& str, const std::string& suffix) {
        if (str.length() < suffix.length())
            return false;
        return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
    };

    while (check(TokenType::Const) || check(TokenType::Static) || check(TokenType::Unsigned) ||
           check(TokenType::Signed) || check(TokenType::Long) || check(TokenType::Short) ||
           check(TokenType::Void) || check(TokenType::Int) || check(TokenType::Bool) ||
           check(TokenType::Char) || check(TokenType::Float) || check(TokenType::Double) ||
           check(TokenType::Auto) || check(TokenType::Identifier) || check(TokenType::Scope) ||
           check(TokenType::Less) || check(TokenType::Greater) || check(TokenType::Comma) ||
           check(TokenType::Asterisk) || check(TokenType::Ampersand)) {
        // Add space before token if not empty and not following scope operator
        if (!type.empty() && !ends_with(type, "::") && !ends_with(type, "<") &&
            current_token().text != "::" && current_token().text != "*" &&
            current_token().text != "&" && current_token().text != "<" &&
            current_token().text != ">") {
            type += " ";
        }

        type += current_token().text;

        // Track if we've seen a base type (not just modifiers)
        if (check(TokenType::Void) || check(TokenType::Int) || check(TokenType::Bool) ||
            check(TokenType::Char) || check(TokenType::Float) || check(TokenType::Double) ||
            check(TokenType::Auto) || check(TokenType::Identifier)) {
            has_base_type = true;
        }

        if (check(TokenType::Less)) {
            type += "<";
            advance();
            int depth = 1;
            while (depth > 0 && !check(TokenType::Eof)) {
                if (check(TokenType::Less)) {
                    type += "<";
                    depth++;
                } else if (check(TokenType::Greater)) {
                    type += ">";
                    depth--;
                } else {
                    type += current_token().text;
                }
                advance();
            }
            has_base_type = true;
        } else {
            advance();
        }

        // If we have a base type and the next token is an identifier (not ::),
        // it's probably the variable/function name, not part of the type
        if (has_base_type && check(TokenType::Identifier) && !ends_with(type, "::")) {
            break;
        }
    }

    return type;
}

void BuiltinParser::skip_to_semicolon() {
    while (!check(TokenType::Semicolon) && !check(TokenType::Eof)) {
        if (check(TokenType::LeftBrace)) {
            skip_braces();
        } else {
            advance();
        }
    }
}

void BuiltinParser::skip_braces() {
    if (!match(TokenType::LeftBrace)) {
        return;
    }

    int depth = 1;
    while (depth > 0 && !check(TokenType::Eof)) {
        if (match(TokenType::LeftBrace)) {
            depth++;
        } else if (match(TokenType::RightBrace)) {
            depth--;
        } else {
            advance();
        }
    }
}

void BuiltinParser::add_error(const std::string& message) {
    const auto& tok = current_token();
    std::string error = filename_ + ":" + std::to_string(tok.line) + ":" +
                        std::to_string(tok.column) + ": " + message;
    errors_.push_back(error);
}

SourcePosition BuiltinParser::get_position() const {
    const auto& tok = current_token();
    return SourcePosition(filename_, tok.line, tok.column);
}

}  // namespace parser
}  // namespace cclint
