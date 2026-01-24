#include "parser/simple_parser.hpp"

namespace cclint {
namespace parser {

SimpleParser::SimpleParser(const std::string& source, const std::string& filename)
    : filename_(filename) {
    Lexer lexer(source);
    tokens_ = lexer.tokenize();
}

std::shared_ptr<TranslationUnitNode> SimpleParser::parse() {
    auto root = std::make_shared<TranslationUnitNode>();
    root->name = filename_;

    while (!check(TokenType::Eof)) {
        parse_toplevel(*root);
    }

    return root;
}

const Token& SimpleParser::current_token() const {
    if (current_ < tokens_.size()) {
        return tokens_[current_];
    }
    static Token eof_token(TokenType::Eof, "", 0, 0);
    return eof_token;
}

const Token& SimpleParser::peek_token(int offset) const {
    size_t pos = current_ + offset;
    if (pos < tokens_.size()) {
        return tokens_[pos];
    }
    static Token eof_token(TokenType::Eof, "", 0, 0);
    return eof_token;
}

bool SimpleParser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool SimpleParser::check(TokenType type) const {
    return current_token().type == type;
}

Token SimpleParser::advance() {
    if (current_ < tokens_.size()) {
        return tokens_[current_++];
    }
    return Token(TokenType::Eof, "", 0, 0);
}

Token SimpleParser::expect(TokenType type, const std::string& message) {
    if (check(type)) {
        return advance();
    }
    add_error(message);
    return current_token();
}

void SimpleParser::parse_toplevel(TranslationUnitNode& root) {
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

    // template (簡易的にスキップ)
    if (check(TokenType::Template)) {
        advance();
        skip_to_semicolon();
        return;
    }

    // 関数または変数
    auto node = parse_function_or_variable();
    if (node) {
        root.children.push_back(node);
    }
}

std::shared_ptr<ASTNode> SimpleParser::parse_namespace() {
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

std::shared_ptr<ASTNode> SimpleParser::parse_class_or_struct() {
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
                node->base_classes.push_back(advance().text);
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

        // メンバ関数または変数
        auto member = parse_function_or_variable();
        if (member) {
            if (auto func = std::dynamic_pointer_cast<FunctionNode>(member)) {
                func->access = current_access_;
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

std::shared_ptr<ASTNode> SimpleParser::parse_enum() {
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

std::shared_ptr<ASTNode> SimpleParser::parse_function_or_variable() {
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

    // 型
    std::string type_name = parse_type();

    // 名前
    std::string name;
    if (check(TokenType::Identifier)) {
        name = advance().text;
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
        if (match(TokenType::LeftBrace)) {
            skip_braces();
        } else {
            match(TokenType::Semicolon);
        }

        return func;
    } else {
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

std::shared_ptr<ASTNode> SimpleParser::parse_typedef() {
    auto node = std::make_shared<TypedefNode>();
    node->position = get_position();

    advance();  // 'typedef'

    skip_to_semicolon();
    match(TokenType::Semicolon);

    return node;
}

std::shared_ptr<ASTNode> SimpleParser::parse_using() {
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

std::string SimpleParser::parse_type() {
    std::string type;

    while (check(TokenType::Const) || check(TokenType::Static) || check(TokenType::Unsigned) ||
           check(TokenType::Signed) || check(TokenType::Long) || check(TokenType::Short) ||
           check(TokenType::Void) || check(TokenType::Int) || check(TokenType::Bool) ||
           check(TokenType::Char) || check(TokenType::Float) || check(TokenType::Double) ||
           check(TokenType::Auto) || check(TokenType::Identifier) || check(TokenType::Scope) ||
           check(TokenType::Less) || check(TokenType::Greater) || check(TokenType::Comma) ||
           check(TokenType::Asterisk) || check(TokenType::Ampersand)) {
        type += current_token().text;
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
        } else {
            advance();
        }
    }

    return type;
}

void SimpleParser::skip_to_semicolon() {
    while (!check(TokenType::Semicolon) && !check(TokenType::Eof)) {
        if (check(TokenType::LeftBrace)) {
            skip_braces();
        } else {
            advance();
        }
    }
}

void SimpleParser::skip_braces() {
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

void SimpleParser::add_error(const std::string& message) {
    const auto& tok = current_token();
    std::string error = filename_ + ":" + std::to_string(tok.line) + ":" +
                        std::to_string(tok.column) + ": " + message;
    errors_.push_back(error);
}

SourcePosition SimpleParser::get_position() const {
    const auto& tok = current_token();
    return SourcePosition(filename_, tok.line, tok.column);
}

}  // namespace parser
}  // namespace cclint
