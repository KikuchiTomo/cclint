#include "parser/builtin_parser.hpp"

namespace cclint {
namespace parser {

BuiltinParser::BuiltinParser(const std::string& source, const std::string& filename,
                             bool use_preprocessor)
    : filename_(filename.empty() ? "<stdin>" : filename) {
    std::cerr << "[DEBUG] BuiltinParser: constructor start, use_preprocessor=" << use_preprocessor << std::endl;
    if (use_preprocessor) {
        std::cerr << "[DEBUG] BuiltinParser: creating Preprocessor..." << std::endl;
        // Use preprocessor for macro expansion and include processing
        Preprocessor preprocessor(source, filename);
        std::cerr << "[DEBUG] BuiltinParser: Preprocessor created, calling preprocess..." << std::endl;
        // Note: Preprocessor defaults to linter mode (no expansion)
        // For full parsing, you may want to enable expansion
        tokens_ = preprocessor.preprocess();
        std::cerr << "[DEBUG] BuiltinParser: preprocess() returned " << tokens_.size() << " tokens" << std::endl;

        if (preprocessor.has_errors()) {
            for (const auto& error : preprocessor.errors()) {
                errors_.push_back(error);
            }
        }
        std::cerr << "[DEBUG] BuiltinParser: preprocessor path complete" << std::endl;
    } else {
        std::cerr << "[DEBUG] BuiltinParser: creating EnhancedLexer..." << std::endl;
        // Direct lexing without preprocessing
        EnhancedLexer lexer(source, filename);
        std::cerr << "[DEBUG] BuiltinParser: calling tokenize..." << std::endl;
        tokens_ = lexer.tokenize();
        std::cerr << "[DEBUG] BuiltinParser: tokenize() returned " << tokens_.size() << " tokens" << std::endl;

        if (lexer.has_errors()) {
            for (const auto& error : lexer.errors()) {
                errors_.push_back(error);
            }
        }
        std::cerr << "[DEBUG] BuiltinParser: lexer path complete" << std::endl;
    }
    std::cerr << "[DEBUG] BuiltinParser: constructor end" << std::endl;
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
    static Token eof_token;
    eof_token.type = TokenType::Eof;
    eof_token.text = "";
    return eof_token;
}

const Token& BuiltinParser::peek_token(int offset) const {
    size_t pos = current_ + offset;
    if (pos < tokens_.size()) {
        return tokens_[pos];
    }
    static Token eof_token;
    eof_token.type = TokenType::Eof;
    eof_token.text = "";
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
    Token eof_token;
    eof_token.type = TokenType::Eof;
    eof_token.text = "";
    return eof_token;
}

Token BuiltinParser::expect(TokenType type, const std::string& message) {
    if (check(type)) {
        return advance();
    }
    add_error(message);
    return current_token();
}

void BuiltinParser::parse_toplevel(TranslationUnitNode& root) {
    // コメントとプリプロセッサディレクティブをスキップ
    while (current_token().type == TokenType::LineComment ||
           current_token().type == TokenType::BlockComment ||
           (current_token().type >= TokenType::PPInclude &&
            current_token().type <= TokenType::PPLine)) {
        advance();
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
    } else {
        // パースに失敗した場合、無限ループを避けるため次のトークンに進む
        if (!check(TokenType::Eof)) {
            advance();
        }
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
        while (match(TokenType::LineComment) || match(TokenType::PPDirective)) {
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
        } else {
            // パースに失敗した場合、無限ループを避けるため次のトークンに進む
            if (!check(TokenType::RightBrace) && !check(TokenType::Eof)) {
                advance();
            }
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
        if (match(TokenType::LineComment) || match(TokenType::PPDirective)) {
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
            } else if (check(TokenType::Identifier) && current_token().text == "override") {
                func->is_override = true;
                advance();
            } else if (check(TokenType::Identifier) && current_token().text == "final") {
                func->is_final = true;
                advance();
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
           check(TokenType::Auto) || check(TokenType::Identifier) ||
           check(TokenType::DoubleColon) || check(TokenType::Less) || check(TokenType::Greater) ||
           check(TokenType::Comma) || check(TokenType::Star) || check(TokenType::Ampersand)) {
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

// ==================== Expression Parsing ====================
// Precedence climbing expression parser
// Supports all C++ operators with correct precedence

std::shared_ptr<ASTNode> BuiltinParser::parse_expression() {
    return parse_assignment_expression();
}

std::shared_ptr<ASTNode> BuiltinParser::parse_assignment_expression() {
    auto left = parse_conditional_expression();

    // Check for assignment operators: =, +=, -=, *=, /=, %=, etc.
    while (check(TokenType::Equal) || current_token().text == "+=" ||
           current_token().text == "-=" || current_token().text == "*=" ||
           current_token().text == "/=" || current_token().text == "%=") {
        std::string op = current_token().text;
        advance();
        auto right = parse_conditional_expression();

        auto expr = std::make_shared<ASTNode>(ASTNodeType::Variable);
        expr->name = "assignment:" + op;
        if (left)
            expr->children.push_back(left);
        if (right)
            expr->children.push_back(right);
        left = expr;
    }

    return left;
}

std::shared_ptr<ASTNode> BuiltinParser::parse_conditional_expression() {
    auto cond = parse_logical_or_expression();

    // Ternary operator: condition ? true_expr : false_expr
    if (match(TokenType::Question)) {
        auto true_expr = parse_expression();
        expect(TokenType::Colon, "Expected ':' in ternary operator");
        auto false_expr = parse_conditional_expression();

        auto expr = std::make_shared<ASTNode>(ASTNodeType::Variable);
        expr->type = ASTNodeType::Variable;
        expr->name = "ternary";
        if (cond)
            expr->children.push_back(cond);
        if (true_expr)
            expr->children.push_back(true_expr);
        if (false_expr)
            expr->children.push_back(false_expr);
        return expr;
    }

    return cond;
}

std::shared_ptr<ASTNode> BuiltinParser::parse_logical_or_expression() {
    auto left = parse_logical_and_expression();

    while (current_token().text == "||") {
        advance();
        auto right = parse_logical_and_expression();

        auto expr = std::make_shared<ASTNode>(ASTNodeType::Variable);
        expr->type = ASTNodeType::Variable;
        expr->name = "logical_or";
        if (left)
            expr->children.push_back(left);
        if (right)
            expr->children.push_back(right);
        left = expr;
    }

    return left;
}

std::shared_ptr<ASTNode> BuiltinParser::parse_logical_and_expression() {
    auto left = parse_equality_expression();

    while (current_token().text == "&&") {
        advance();
        auto right = parse_equality_expression();

        auto expr = std::make_shared<ASTNode>(ASTNodeType::Variable);
        expr->type = ASTNodeType::Variable;
        expr->name = "logical_and";
        if (left)
            expr->children.push_back(left);
        if (right)
            expr->children.push_back(right);
        left = expr;
    }

    return left;
}

std::shared_ptr<ASTNode> BuiltinParser::parse_equality_expression() {
    auto left = parse_relational_expression();

    while (current_token().text == "==" || current_token().text == "!=") {
        std::string op = current_token().text;
        advance();
        auto right = parse_relational_expression();

        auto expr = std::make_shared<ASTNode>(ASTNodeType::Variable);
        expr->type = ASTNodeType::Variable;
        expr->name = "equality:" + op;
        if (left)
            expr->children.push_back(left);
        if (right)
            expr->children.push_back(right);
        left = expr;
    }

    return left;
}

std::shared_ptr<ASTNode> BuiltinParser::parse_relational_expression() {
    auto left = parse_additive_expression();

    while (check(TokenType::Less) || check(TokenType::Greater) || current_token().text == "<=" ||
           current_token().text == ">=" ||
           current_token().text == "<=>") {  // C++20 spaceship operator
        std::string op = current_token().text;
        advance();
        auto right = parse_additive_expression();

        auto expr = std::make_shared<ASTNode>(ASTNodeType::Variable);
        expr->type = ASTNodeType::Variable;
        expr->name = "relational:" + op;
        if (left)
            expr->children.push_back(left);
        if (right)
            expr->children.push_back(right);
        left = expr;
    }

    return left;
}

std::shared_ptr<ASTNode> BuiltinParser::parse_additive_expression() {
    auto left = parse_multiplicative_expression();

    while (check(TokenType::Plus) || check(TokenType::Minus)) {
        std::string op = current_token().text;
        advance();
        auto right = parse_multiplicative_expression();

        auto expr = std::make_shared<ASTNode>(ASTNodeType::Variable);
        expr->type = ASTNodeType::Variable;
        expr->name = "additive:" + op;
        if (left)
            expr->children.push_back(left);
        if (right)
            expr->children.push_back(right);
        left = expr;
    }

    return left;
}

std::shared_ptr<ASTNode> BuiltinParser::parse_multiplicative_expression() {
    auto left = parse_unary_expression();

    while (check(TokenType::Star) || check(TokenType::Slash) || check(TokenType::Percent)) {
        std::string op = current_token().text;
        advance();
        auto right = parse_unary_expression();

        auto expr = std::make_shared<ASTNode>(ASTNodeType::Variable);
        expr->type = ASTNodeType::Variable;
        expr->name = "multiplicative:" + op;
        if (left)
            expr->children.push_back(left);
        if (right)
            expr->children.push_back(right);
        left = expr;
    }

    return left;
}

std::shared_ptr<ASTNode> BuiltinParser::parse_unary_expression() {
    // Unary operators: ++, --, !, ~, -, +, *, &, sizeof, alignof, new, delete
    if (current_token().text == "++" || current_token().text == "--" ||
        check(TokenType::LogicalNot) || check(TokenType::Tilde) || check(TokenType::Minus) ||
        check(TokenType::Plus) || check(TokenType::Star) || check(TokenType::Ampersand) ||
        current_token().text == "sizeof" || current_token().text == "alignof" ||
        check(TokenType::New) || check(TokenType::Delete)) {
        std::string op = current_token().text;
        advance();
        auto operand = parse_unary_expression();

        auto expr = std::make_shared<ASTNode>(ASTNodeType::Variable);
        expr->type = ASTNodeType::Variable;
        expr->name = "unary:" + op;
        if (operand)
            expr->children.push_back(operand);
        return expr;
    }

    // Cast expressions: static_cast, dynamic_cast, const_cast, reinterpret_cast
    if (current_token().text == "static_cast" || current_token().text == "dynamic_cast" ||
        current_token().text == "const_cast" || current_token().text == "reinterpret_cast") {
        std::string cast_type = current_token().text;
        advance();

        // Skip template arguments <type>
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

        // Parse expression in parentheses
        if (match(TokenType::LeftParen)) {
            auto operand = parse_expression();
            expect(TokenType::RightParen, "Expected ')' after cast expression");

            auto expr = std::make_shared<ASTNode>(ASTNodeType::Variable);
            expr->type = ASTNodeType::Variable;
            expr->name = "cast:" + cast_type;
            if (operand)
                expr->children.push_back(operand);
            return expr;
        }
    }

    return parse_postfix_expression();
}

std::shared_ptr<ASTNode> BuiltinParser::parse_postfix_expression() {
    auto expr = parse_primary_expression();

    while (true) {
        // Array subscript: expr[index]
        if (match(TokenType::LeftBracket)) {
            auto index = parse_expression();
            expect(TokenType::RightBracket, "Expected ']' after array subscript");

            auto subscript = std::make_shared<ASTNode>(ASTNodeType::Variable);
            subscript->type = ASTNodeType::Variable;
            subscript->name = "subscript";
            if (expr)
                subscript->children.push_back(expr);
            if (index)
                subscript->children.push_back(index);
            expr = subscript;
        }
        // Function call: expr(args...)
        else if (match(TokenType::LeftParen)) {
            auto call = std::make_shared<CallExpressionNode>();
            call->position = get_position();
            if (expr && expr->type == ASTNodeType::Variable) {
                call->function_name = expr->name;
            }

            // Parse arguments
            while (!check(TokenType::RightParen) && !check(TokenType::Eof)) {
                auto arg = parse_assignment_expression();
                if (arg)
                    call->children.push_back(arg);
                if (!match(TokenType::Comma))
                    break;
            }

            expect(TokenType::RightParen, "Expected ')' after function arguments");
            expr = call;
        }
        // Member access: expr.member or expr->member
        else if (match(TokenType::Dot) || current_token().text == "->") {
            std::string op = current_token().text;
            if (op == "->")
                advance();

            if (check(TokenType::Identifier)) {
                auto member = std::make_shared<ASTNode>(ASTNodeType::Variable);
                member->type = ASTNodeType::Variable;
                member->name = "member_access:" + op + ":" + advance().text;
                if (expr)
                    member->children.push_back(expr);
                expr = member;
            }
        }
        // Postfix increment/decrement: expr++ or expr--
        else if (current_token().text == "++" || current_token().text == "--") {
            std::string op = current_token().text;
            advance();

            auto postfix = std::make_shared<ASTNode>(ASTNodeType::Variable);
            postfix->type = ASTNodeType::Variable;
            postfix->name = "postfix:" + op;
            if (expr)
                postfix->children.push_back(expr);
            expr = postfix;
        } else {
            break;
        }
    }

    return expr;
}

std::shared_ptr<ASTNode> BuiltinParser::parse_primary_expression() {
    // Literals: numbers, strings, characters, booleans, nullptr
    if (check(TokenType::IntegerLiteral) || check(TokenType::FloatingLiteral) ||
        check(TokenType::StringLiteral) || check(TokenType::CharLiteral) ||
        check(TokenType::True) || check(TokenType::False) || check(TokenType::Nullptr)) {
        auto literal = std::make_shared<ASTNode>(ASTNodeType::Variable);
        literal->type = ASTNodeType::Variable;
        literal->name = "literal:" + current_token().text;
        advance();
        return literal;
    }

    // Identifiers
    if (check(TokenType::Identifier)) {
        auto ident = std::make_shared<ASTNode>(ASTNodeType::Variable);
        ident->type = ASTNodeType::Variable;
        ident->name = advance().text;
        return ident;
    }

    // Parenthesized expression
    if (match(TokenType::LeftParen)) {
        auto expr = parse_expression();
        expect(TokenType::RightParen, "Expected ')' after expression");
        return expr;
    }

    // Lambda expression: [...](...){}
    if (check(TokenType::LeftBracket)) {
        auto lambda = std::make_shared<LambdaNode>();
        lambda->position = get_position();

        // Parse capture clause
        advance();  // [
        std::string capture;
        while (!check(TokenType::RightBracket) && !check(TokenType::Eof)) {
            capture += current_token().text;
            if (current_token().text == "=")
                lambda->captures_by_value = true;
            if (current_token().text == "&")
                lambda->captures_by_reference = true;
            advance();
        }
        lambda->capture_clause = "[" + capture + "]";
        expect(TokenType::RightBracket, "Expected ']' after lambda capture");

        // Parse parameters (optional)
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

        // Parse mutable/constexpr
        if (current_token().text == "mutable") {
            lambda->is_mutable = true;
            advance();
        }
        if (current_token().text == "constexpr") {
            lambda->is_constexpr = true;
            advance();
        }

        // Parse body
        if (check(TokenType::LeftBrace)) {
            skip_braces();
        }

        return lambda;
    }

    // This or other keywords
    if (check(TokenType::This)) {
        auto expr = std::make_shared<ASTNode>(ASTNodeType::Variable);
        expr->type = ASTNodeType::Variable;
        expr->name = "this";
        advance();
        return expr;
    }

    return nullptr;
}

// ==================== Statement Parsing ====================

std::shared_ptr<ASTNode> BuiltinParser::parse_statement() {
    // Compound statement: { ... }
    if (check(TokenType::LeftBrace)) {
        return parse_compound_statement();
    }

    // If statement
    if (check(TokenType::If)) {
        return parse_if_statement();
    }

    // Switch statement
    if (check(TokenType::Switch)) {
        return parse_switch_statement();
    }

    // For loop
    if (check(TokenType::For)) {
        return parse_for_statement();
    }

    // While loop
    if (check(TokenType::While)) {
        return parse_while_statement();
    }

    // Do-while loop
    if (check(TokenType::Do)) {
        return parse_do_while_statement();
    }

    // Return statement
    if (check(TokenType::Return)) {
        return parse_return_statement();
    }

    // Try-catch block
    if (check(TokenType::Try)) {
        return parse_try_statement();
    }

    // Break statement
    if (check(TokenType::Break)) {
        auto stmt = std::make_shared<ASTNode>(ASTNodeType::Variable);
        stmt->type = ASTNodeType::Variable;
        stmt->name = "break";
        advance();
        match(TokenType::Semicolon);
        return stmt;
    }

    // Continue statement
    if (check(TokenType::Continue)) {
        auto stmt = std::make_shared<ASTNode>(ASTNodeType::Variable);
        stmt->type = ASTNodeType::Variable;
        stmt->name = "continue";
        advance();
        match(TokenType::Semicolon);
        return stmt;
    }

    // Goto statement
    if (check(TokenType::Goto)) {
        auto stmt = std::make_shared<ASTNode>(ASTNodeType::Variable);
        stmt->type = ASTNodeType::Variable;
        stmt->name = "goto";
        advance();
        if (check(TokenType::Identifier)) {
            stmt->name += ":" + advance().text;
        }
        match(TokenType::Semicolon);
        return stmt;
    }

    // Expression statement or declaration
    auto expr = parse_expression();
    match(TokenType::Semicolon);
    return expr;
}

std::shared_ptr<ASTNode> BuiltinParser::parse_compound_statement() {
    auto compound = std::make_shared<ASTNode>(ASTNodeType::Variable);
    compound->type = ASTNodeType::Variable;
    compound->name = "compound_statement";
    compound->position = get_position();

    expect(TokenType::LeftBrace, "Expected '{'");

    while (!check(TokenType::RightBrace) && !check(TokenType::Eof)) {
        auto stmt = parse_statement();
        if (stmt)
            compound->children.push_back(stmt);
    }

    expect(TokenType::RightBrace, "Expected '}'");
    return compound;
}

std::shared_ptr<ASTNode> BuiltinParser::parse_if_statement() {
    auto if_stmt = std::make_shared<ASTNode>(ASTNodeType::Variable);
    if_stmt->type = ASTNodeType::Variable;
    if_stmt->name = "if_statement";
    if_stmt->position = get_position();

    advance();  // consume 'if'

    // C++17: if constexpr
    if (current_token().text == "constexpr") {
        if_stmt->name = "if_constexpr";
        advance();
    }

    // C++17: init-statement
    // if (init; condition) { }

    expect(TokenType::LeftParen, "Expected '(' after 'if'");

    auto condition = parse_expression();
    if (condition)
        if_stmt->children.push_back(condition);

    expect(TokenType::RightParen, "Expected ')' after condition");

    auto then_stmt = parse_statement();
    if (then_stmt)
        if_stmt->children.push_back(then_stmt);

    // Else clause
    if (check(TokenType::Else)) {
        advance();
        auto else_stmt = parse_statement();
        if (else_stmt)
            if_stmt->children.push_back(else_stmt);
    }

    return if_stmt;
}

std::shared_ptr<ASTNode> BuiltinParser::parse_switch_statement() {
    auto switch_stmt = std::make_shared<ASTNode>(ASTNodeType::Variable);
    switch_stmt->type = ASTNodeType::Variable;
    switch_stmt->name = "switch_statement";
    switch_stmt->position = get_position();

    advance();  // consume 'switch'

    expect(TokenType::LeftParen, "Expected '(' after 'switch'");
    auto condition = parse_expression();
    if (condition)
        switch_stmt->children.push_back(condition);
    expect(TokenType::RightParen, "Expected ')' after switch condition");

    auto body = parse_compound_statement();
    if (body)
        switch_stmt->children.push_back(body);

    return switch_stmt;
}

std::shared_ptr<ASTNode> BuiltinParser::parse_for_statement() {
    auto for_stmt = std::make_shared<ASTNode>(ASTNodeType::Variable);
    for_stmt->type = ASTNodeType::Variable;
    for_stmt->name = "for_statement";
    for_stmt->position = get_position();

    advance();  // consume 'for'

    expect(TokenType::LeftParen, "Expected '(' after 'for'");

    // Check for range-based for: for (decl : range)
    size_t save_pos = current_;
    bool is_range_based = false;

    // Try to detect range-based for by looking for ':' before ';'
    while (!check(TokenType::Eof) && !check(TokenType::RightParen)) {
        if (check(TokenType::Colon)) {
            is_range_based = true;
            break;
        }
        if (check(TokenType::Semicolon)) {
            break;
        }
        advance();
    }

    current_ = save_pos;  // Restore position

    if (is_range_based) {
        for_stmt->name = "range_for_statement";

        // Parse declaration
        while (!check(TokenType::Colon) && !check(TokenType::Eof)) {
            advance();
        }
        expect(TokenType::Colon, "Expected ':' in range-based for");

        // Parse range expression
        auto range = parse_expression();
        if (range)
            for_stmt->children.push_back(range);
    } else {
        // Traditional for: for (init; condition; increment)

        // Init
        if (!check(TokenType::Semicolon)) {
            auto init = parse_expression();
            if (init)
                for_stmt->children.push_back(init);
        }
        expect(TokenType::Semicolon, "Expected ';' after for init");

        // Condition
        if (!check(TokenType::Semicolon)) {
            auto cond = parse_expression();
            if (cond)
                for_stmt->children.push_back(cond);
        }
        expect(TokenType::Semicolon, "Expected ';' after for condition");

        // Increment
        if (!check(TokenType::RightParen)) {
            auto inc = parse_expression();
            if (inc)
                for_stmt->children.push_back(inc);
        }
    }

    expect(TokenType::RightParen, "Expected ')' after for");

    auto body = parse_statement();
    if (body)
        for_stmt->children.push_back(body);

    return for_stmt;
}

std::shared_ptr<ASTNode> BuiltinParser::parse_while_statement() {
    auto while_stmt = std::make_shared<ASTNode>(ASTNodeType::Variable);
    while_stmt->type = ASTNodeType::Variable;
    while_stmt->name = "while_statement";
    while_stmt->position = get_position();

    advance();  // consume 'while'

    expect(TokenType::LeftParen, "Expected '(' after 'while'");
    auto condition = parse_expression();
    if (condition)
        while_stmt->children.push_back(condition);
    expect(TokenType::RightParen, "Expected ')' after while condition");

    auto body = parse_statement();
    if (body)
        while_stmt->children.push_back(body);

    return while_stmt;
}

std::shared_ptr<ASTNode> BuiltinParser::parse_do_while_statement() {
    auto do_while_stmt = std::make_shared<ASTNode>(ASTNodeType::Variable);
    do_while_stmt->type = ASTNodeType::Variable;
    do_while_stmt->name = "do_while_statement";
    do_while_stmt->position = get_position();

    advance();  // consume 'do'

    auto body = parse_statement();
    if (body)
        do_while_stmt->children.push_back(body);

    expect(TokenType::While, "Expected 'while' after do body");
    expect(TokenType::LeftParen, "Expected '(' after 'while'");
    auto condition = parse_expression();
    if (condition)
        do_while_stmt->children.push_back(condition);
    expect(TokenType::RightParen, "Expected ')' after while condition");
    match(TokenType::Semicolon);

    return do_while_stmt;
}

std::shared_ptr<ASTNode> BuiltinParser::parse_return_statement() {
    auto return_stmt = std::make_shared<ASTNode>(ASTNodeType::Variable);
    return_stmt->type = ASTNodeType::Variable;
    return_stmt->name = "return_statement";
    return_stmt->position = get_position();

    advance();  // consume 'return'

    if (!check(TokenType::Semicolon)) {
        auto expr = parse_expression();
        if (expr)
            return_stmt->children.push_back(expr);
    }

    match(TokenType::Semicolon);
    return return_stmt;
}

std::shared_ptr<ASTNode> BuiltinParser::parse_try_statement() {
    auto try_stmt = std::make_shared<ASTNode>(ASTNodeType::Variable);
    try_stmt->type = ASTNodeType::Variable;
    try_stmt->name = "try_statement";
    try_stmt->position = get_position();

    advance();  // consume 'try'

    auto try_block = parse_compound_statement();
    if (try_block)
        try_stmt->children.push_back(try_block);

    // Parse catch clauses
    while (check(TokenType::Catch)) {
        auto catch_clause = std::make_shared<ASTNode>(ASTNodeType::Variable);
        catch_clause->type = ASTNodeType::Variable;
        catch_clause->name = "catch_clause";
        catch_clause->position = get_position();

        advance();  // consume 'catch'

        expect(TokenType::LeftParen, "Expected '(' after 'catch'");
        // Parse exception declaration (skip for now)
        while (!check(TokenType::RightParen) && !check(TokenType::Eof)) {
            advance();
        }
        expect(TokenType::RightParen, "Expected ')' after catch parameter");

        auto catch_body = parse_compound_statement();
        if (catch_body)
            catch_clause->children.push_back(catch_body);

        try_stmt->children.push_back(catch_clause);
    }

    return try_stmt;
}

std::shared_ptr<ASTNode> BuiltinParser::parse_declaration_statement() {
    // For now, parse as expression and skip to semicolon
    auto decl = parse_expression();
    match(TokenType::Semicolon);
    return decl;
}

}  // namespace parser
}  // namespace cclint
